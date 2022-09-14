#include "solver/fp/word_blaster.h"

extern "C" {
#include "bzlafp.h"
}

#include <sstream>

#include "solver/fp/symfpu_wrapper.h"
#include "symfpu/core/classify.h"
#include "symfpu/core/compare.h"
#include "symfpu/core/convert.h"
#include "symfpu/core/divide.h"
#include "symfpu/core/fma.h"
#include "symfpu/core/packing.h"
#include "symfpu/core/remainder.h"
#include "symfpu/core/sqrt.h"
#include "symfpu/core/unpackedFloat.h"

namespace bzla {
namespace fp {

namespace {
std::string
create_component_symbol(BzlaNode *node, const char *s)
{
  assert(node);
  assert(s);
  std::stringstream ss;
  ss << "_fp_var_" << bzla_node_get_id(node) << s << "_component_";
  return ss.str();
}
}  // namespace

struct WordBlaster::Internal
{
  SymFpuSymRMMap d_rm_map;
  SymFpuSymPropMap d_prop_map;
  SymUBVMap d_ubv_map;
  SymSBVMap d_sbv_map;
  UnpackedFloatMap d_unpacked_float_map;
  PackedFloatMap d_packed_float_map;
};

/* --- WordBlaster public --------------------------------------------------- */

WordBlaster::WordBlaster(Bzla *bzla) : d_bzla(bzla)
{
  d_internal.reset(new Internal());
}

WordBlaster::~WordBlaster()
{
  for (const auto &p : d_min_max_uf_map)
  {
    bzla_sort_release(d_bzla, p.first);
    bzla_node_release(d_bzla, p.second);
  }
  for (const auto &p : d_sbv_ubv_uf_map)
  {
    bzla_sort_release(d_bzla, p.first.first);
    bzla_sort_release(d_bzla, p.first.second);
    bzla_node_release(d_bzla, p.second);
  }
  for (const auto &p : d_internal->d_unpacked_float_map)
  {
    bzla_node_release(d_bzla, p.first);
  }
  for (const auto &p : d_internal->d_rm_map)
  {
    bzla_node_release(d_bzla, p.first);
  }
  for (const auto &p : d_internal->d_prop_map)
  {
    bzla_node_release(d_bzla, p.first);
  }
  for (const auto &p : d_internal->d_ubv_map)
  {
    bzla_node_release(d_bzla, p.first);
  }
  for (const auto &p : d_internal->d_sbv_map)
  {
    bzla_node_release(d_bzla, p.first);
  }
  for (BzlaNode *node : d_additional_assertions)
  {
    bzla_node_release(d_bzla, node);
  }
}

void
WordBlaster::set_s_bzla(Bzla *bzla)
{
  FloatingPoint::s_bzla         = bzla;
  FloatingPointTypeInfo::s_bzla = bzla;
  SymFpuSymRM::s_bzla           = bzla;
  SymFpuSymProp::s_bzla         = bzla;
  SymFpuSymBV<true>::s_bzla     = bzla;
  SymFpuSymBV<false>::s_bzla    = bzla;
}

BzlaNode *
WordBlaster::word_blast(BzlaNode *node)
{
  assert(d_bzla);
  assert(node);
  assert(bzla_node_is_regular(node));
  assert(d_bzla == bzla_node_real_addr(node)->bzla);
  assert((bzla_node_is_bv(d_bzla, node) && node->arity
          && (bzla_node_is_fp(d_bzla, node->e[0])
              || bzla_node_is_rm(d_bzla, node->e[0])))
         || bzla_node_is_fp(d_bzla, node) || bzla_node_is_rm(d_bzla, node));

  BzlaNode *res = nullptr;

  BzlaNode *cur;
  std::vector<BzlaNode *> to_visit;
  std::unordered_map<BzlaNode *, uint32_t, BzlaNodeHashFunction> visited;

  to_visit.push_back(node);

  while (!to_visit.empty())
  {
    cur = bzla_node_real_addr(to_visit.back());
    assert(!bzla_node_real_addr(cur)->parameterized);
    to_visit.pop_back();

    if (d_internal->d_prop_map.find(cur) != d_internal->d_prop_map.end()
        || d_internal->d_rm_map.find(cur) != d_internal->d_rm_map.end()
        || d_internal->d_sbv_map.find(cur) != d_internal->d_sbv_map.end()
        || d_internal->d_ubv_map.find(cur) != d_internal->d_ubv_map.end()
        || d_internal->d_unpacked_float_map.find(cur)
               != d_internal->d_unpacked_float_map.end())
    {
      continue;
    }

    if (visited.find(cur) == visited.end())
    {
      visited.emplace(cur, 0);
      to_visit.push_back(cur);

      /* We treat applies and quantifiers as variables. */
      if (!bzla_node_is_apply(cur) && !bzla_node_is_forall(cur))
      {
        for (uint32_t i = 0; i < cur->arity; ++i)
        {
          to_visit.push_back(cur->e[i]);
        }
      }
    }
    else if (visited.at(cur) == 0)
    {
      if (bzla_node_is_cond(cur) && bzla_node_is_rm(d_bzla, cur->e[1]))
      {
        assert(d_internal->d_rm_map.find(cur->e[1])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_rm_map.find(cur->e[2])
               != d_internal->d_rm_map.end());
        d_internal->d_rm_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::ite<SymFpuSymProp, SymFpuSymRM>::iteOp(
                SymFpuSymProp(cur->e[0]),
                d_internal->d_rm_map.at(cur->e[1]),
                d_internal->d_rm_map.at(cur->e[2])));
      }
      else if (bzla_node_is_cond(cur) && bzla_node_is_fp(d_bzla, cur->e[1]))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[2])
               != d_internal->d_unpacked_float_map.end());

        // Consruct ITEs over unpacked float components
        auto uf1 = d_internal->d_unpacked_float_map.at(cur->e[1]);
        auto uf2 = d_internal->d_unpacked_float_map.at(cur->e[2]);

        BzlaNode *nan = bzla_exp_cond(
            d_bzla, cur->e[0], uf1.getNaN().getNode(), uf2.getNaN().getNode());
        BzlaNode *inf = bzla_exp_cond(
            d_bzla, cur->e[0], uf1.getInf().getNode(), uf2.getInf().getNode());
        BzlaNode *zero = bzla_exp_cond(d_bzla,
                                       cur->e[0],
                                       uf1.getZero().getNode(),
                                       uf2.getZero().getNode());
        BzlaNode *sign = bzla_exp_cond(d_bzla,
                                       cur->e[0],
                                       uf1.getSign().getNode(),
                                       uf2.getSign().getNode());
        BzlaNode *exp  = bzla_exp_cond(d_bzla,
                                      cur->e[0],
                                      uf1.getExponent().getNode(),
                                      uf2.getExponent().getNode());
        BzlaNode *sig  = bzla_exp_cond(d_bzla,
                                      cur->e[0],
                                      uf1.getSignificand().getNode(),
                                      uf2.getSignificand().getNode());

        SymUnpackedFloat ite(nan, inf, zero, sign, exp, sig);
        d_internal->d_unpacked_float_map.emplace(bzla_node_copy(d_bzla, cur),
                                                 ite);
        bzla_node_release(d_bzla, nan);
        bzla_node_release(d_bzla, inf);
        bzla_node_release(d_bzla, zero);
        bzla_node_release(d_bzla, sign);
        bzla_node_release(d_bzla, exp);
        bzla_node_release(d_bzla, sig);
      }
      else if (bzla_node_is_rm_const(cur))
      {
        d_internal->d_rm_map.emplace(bzla_node_copy(d_bzla, cur),
                                     SymFpuSymRM(cur));
      }
      else if (bzla_node_is_rm_var(cur)
               || (bzla_node_is_apply(cur) && bzla_node_is_rm(d_bzla, cur)))
      {
        SymFpuSymRM var(cur);
        d_internal->d_rm_map.emplace(bzla_node_copy(d_bzla, cur), var);
        d_additional_assertions.push_back(
            bzla_node_copy(d_bzla, var.valid().getNode()));
      }
      else if (bzla_node_is_fp_const(cur))
      {
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            SymUnpackedFloat(*static_cast<UnpackedFloat *>(
                bzla_fp_get_unpacked_float(cur))));
      }
      else if (bzla_node_is_fp_var(cur)
               || (bzla_node_is_apply(cur) && bzla_node_is_fp(d_bzla, cur)))
      {
        BzlaSortId sort   = bzla_node_get_sort_id(cur);
        BzlaSortId sort_1 = bzla_sort_bv(d_bzla, 1);
        BzlaSortId sort_exp =
            bzla_sort_bv(d_bzla, SymUnpackedFloat::exponentWidth(sort));
        BzlaSortId sort_sig =
            bzla_sort_bv(d_bzla, SymUnpackedFloat::significandWidth(sort));

        BzlaNode *inf = bzla_exp_var(
            d_bzla, sort_1, create_component_symbol(cur, "inf").c_str());
        BzlaNode *nan = bzla_exp_var(
            d_bzla, sort_1, create_component_symbol(cur, "nan").c_str());
        BzlaNode *sign = bzla_exp_var(
            d_bzla, sort_1, create_component_symbol(cur, "sign").c_str());
        BzlaNode *zero = bzla_exp_var(
            d_bzla, sort_1, create_component_symbol(cur, "zero").c_str());
        BzlaNode *exp = bzla_exp_var(
            d_bzla, sort_exp, create_component_symbol(cur, "exp").c_str());
        BzlaNode *sig = bzla_exp_var(
            d_bzla, sort_sig, create_component_symbol(cur, "sig").c_str());

        SymUnpackedFloat uf(nan, inf, zero, sign, exp, sig);
        d_internal->d_unpacked_float_map.emplace(bzla_node_copy(d_bzla, cur),
                                                 uf);
        d_additional_assertions.push_back(
            bzla_node_copy(d_bzla, uf.valid(sort).getNode()));

        bzla_node_release(d_bzla, sig);
        bzla_node_release(d_bzla, exp);
        bzla_node_release(d_bzla, zero);
        bzla_node_release(d_bzla, sign);
        bzla_node_release(d_bzla, nan);
        bzla_node_release(d_bzla, inf);
        bzla_sort_release(d_bzla, sort_sig);
        bzla_sort_release(d_bzla, sort_exp);
        bzla_sort_release(d_bzla, sort_1);
      }
      else if (bzla_node_is_fp_eq(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::smtlibEqual<SymFpuSymTraits>(
                FloatingPointTypeInfo(bzla_node_get_sort_id(cur->e[0])),
                d_internal->d_unpacked_float_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_rm_eq(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_rm_map.find(cur->e[1])
               != d_internal->d_rm_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            d_internal->d_rm_map.at(cur->e[0])
                == d_internal->d_rm_map.at(cur->e[1]));
      }
      else if (bzla_node_is_fp_abs(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::absolute<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_neg(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::negate<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_normal(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isNormal<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_subnormal(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isSubnormal<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_zero(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isZero<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_inf(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isInfinite<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_nan(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isNaN<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_neg(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isNegative<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_is_pos(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::isPositive<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0])));
      }
      else if (bzla_node_is_fp_lte(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::lessThanOrEqual<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_lt(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_prop_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::lessThan<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_min(cur) || bzla_node_is_fp_max(cur))
      {
        assert(cur->arity == 2);
        BzlaNode *uf = min_max_uf(cur);
        BzlaNode *args[2];
        for (uint32_t i = 0; i < cur->arity; ++i)
        {
          assert(d_internal->d_unpacked_float_map.find(cur->e[i])
                 != d_internal->d_unpacked_float_map.end());
          if (d_internal->d_packed_float_map.find(cur->e[i])
              == d_internal->d_packed_float_map.end())
          {
            d_internal->d_packed_float_map.emplace(
                cur->e[i],
                symfpu::pack(bzla_node_get_sort_id(cur->e[i]),
                             d_internal->d_unpacked_float_map.at(cur->e[i])));
          }
          args[i] = d_internal->d_packed_float_map.at(cur->e[i]).getNode();
        }
        BzlaNode *apply_args = bzla_exp_args(d_bzla, args, cur->arity);
        BzlaNode *apply      = bzla_exp_apply(d_bzla, uf, apply_args);
        if (bzla_node_is_fp_min(cur))
        {
          d_internal->d_unpacked_float_map.emplace(
              bzla_node_copy(d_bzla, cur),
              symfpu::min<SymFpuSymTraits>(
                  bzla_node_get_sort_id(cur),
                  d_internal->d_unpacked_float_map.at(cur->e[0]),
                  d_internal->d_unpacked_float_map.at(cur->e[1]),
                  apply));
        }
        else
        {
          d_internal->d_unpacked_float_map.emplace(
              bzla_node_copy(d_bzla, cur),
              symfpu::max<SymFpuSymTraits>(
                  bzla_node_get_sort_id(cur),
                  d_internal->d_unpacked_float_map.at(cur->e[0]),
                  d_internal->d_unpacked_float_map.at(cur->e[1]),
                  apply));
        }
        bzla_node_release(d_bzla, apply);
        bzla_node_release(d_bzla, apply_args);
      }
      else if (bzla_node_is_fp_rem(cur))
      {
        assert(d_internal->d_unpacked_float_map.find(cur->e[0])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::remainder<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_unpacked_float_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_sqrt(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::sqrt<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_rti(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::roundToIntegral<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_add(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[2])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::add<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1]),
                d_internal->d_unpacked_float_map.at(cur->e[2]),
                SymFpuSymProp(true)));
      }
      else if (bzla_node_is_fp_mul(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[2])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::multiply<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1]),
                d_internal->d_unpacked_float_map.at(cur->e[2])));
      }
      else if (bzla_node_is_fp_div(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[2])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::divide<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1]),
                d_internal->d_unpacked_float_map.at(cur->e[2])));
      }
      else if (bzla_node_is_fp_fma(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[2])
               != d_internal->d_unpacked_float_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[3])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::fma<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1]),
                d_internal->d_unpacked_float_map.at(cur->e[2]),
                d_internal->d_unpacked_float_map.at(cur->e[3])));
      }
      else if (bzla_node_is_fp_to_sbv(cur) || bzla_node_is_fp_to_ubv(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        uint32_t bw          = bzla_node_bv_get_width(d_bzla, cur);
        BzlaNode *uf         = sbv_ubv_uf(cur);
        BzlaNode *args[2]    = {cur->e[0], cur->e[1]};
        BzlaNode *apply_args = bzla_exp_args(d_bzla, args, cur->arity);
        BzlaNode *apply      = bzla_exp_apply(d_bzla, uf, apply_args);
        if (bzla_node_is_fp_to_sbv(cur))
        {
          d_internal->d_sbv_map.emplace(
              bzla_node_copy(d_bzla, cur),
              symfpu::convertFloatToSBV<SymFpuSymTraits>(
                  bzla_node_get_sort_id(cur->e[1]),
                  d_internal->d_rm_map.at(cur->e[0]),
                  d_internal->d_unpacked_float_map.at(cur->e[1]),
                  bw,
                  SymFpuSymBV<true>(apply)));
        }
        else
        {
          d_internal->d_ubv_map.emplace(
              bzla_node_copy(d_bzla, cur),
              symfpu::convertFloatToUBV<SymFpuSymTraits>(
                  bzla_node_get_sort_id(cur->e[1]),
                  d_internal->d_rm_map.at(cur->e[0]),
                  d_internal->d_unpacked_float_map.at(cur->e[1]),
                  bw,
                  SymFpuSymBV<false>(apply)));
        }
        bzla_node_release(d_bzla, apply);
        bzla_node_release(d_bzla, apply_args);
      }
      else if (bzla_node_is_fp_to_fp_from_bv(cur))
      {
        assert(bzla_node_is_bv(d_bzla, cur->e[0]));
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::unpack<SymFpuSymTraits>(bzla_node_get_sort_id(cur),
                                            SymFpuSymBV<false>(cur->e[0])));
      }
      else if (bzla_node_is_fp_to_fp_from_fp(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(d_internal->d_unpacked_float_map.find(cur->e[1])
               != d_internal->d_unpacked_float_map.end());
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::convertFloatToFloat<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur->e[1]),
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                d_internal->d_unpacked_float_map.at(cur->e[1])));
      }
      else if (bzla_node_is_fp_to_fp_from_sbv(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(bzla_node_is_bv(d_bzla, cur->e[1]));
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::convertSBVToFloat<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                SymFpuSymBV<true>(cur->e[1])));
      }
      else if (bzla_node_is_fp_to_fp_from_ubv(cur))
      {
        assert(d_internal->d_rm_map.find(cur->e[0])
               != d_internal->d_rm_map.end());
        assert(bzla_node_is_bv(d_bzla, cur->e[1]));
        d_internal->d_unpacked_float_map.emplace(
            bzla_node_copy(d_bzla, cur),
            symfpu::convertUBVToFloat<SymFpuSymTraits>(
                bzla_node_get_sort_id(cur),
                d_internal->d_rm_map.at(cur->e[0]),
                SymFpuSymBV<false>(cur->e[1])));
      }
      visited.at(cur) = 1;
    }
    else
    {
      assert(visited.at(cur) == 1);
      continue;
    }
  }

  if (d_internal->d_prop_map.find(node) != d_internal->d_prop_map.end())
  {
    assert(bzla_sort_is_bool(d_bzla, bzla_node_get_sort_id(node)));
    res = d_internal->d_prop_map.at(node).getNode();
  }
  else if (d_internal->d_rm_map.find(node) != d_internal->d_rm_map.end())
  {
    assert(bzla_node_is_rm(d_bzla, node));
    res = d_internal->d_rm_map.at(node).getNode();
  }
  else if (d_internal->d_sbv_map.find(node) != d_internal->d_sbv_map.end())
  {
    assert(bzla_node_is_fp_to_sbv(node));
    res = d_internal->d_sbv_map.at(node).getNode();
  }
  else if (d_internal->d_ubv_map.find(node) != d_internal->d_ubv_map.end())
  {
    assert(bzla_node_is_fp_to_ubv(node));
    res = d_internal->d_ubv_map.at(node).getNode();
  }
  else
  {
    assert(d_internal->d_unpacked_float_map.find(node)
           != d_internal->d_unpacked_float_map.end());
    d_internal->d_packed_float_map.emplace(
        node,
        symfpu::pack(bzla_node_get_sort_id(node),
                     d_internal->d_unpacked_float_map.at(node)));
    res = d_internal->d_packed_float_map.at(node).getNode();
  }
  assert(res);
  return res;
}

BzlaNode *
WordBlaster::get_word_blasted_node(BzlaNode *node)
{
  assert(d_bzla);
  assert(node);
  assert(bzla_node_is_regular(node));
  assert(d_bzla == node->bzla);

  if (d_internal->d_packed_float_map.find(node)
      != d_internal->d_packed_float_map.end())
  {
    return d_internal->d_packed_float_map.at(node).getNode();
  }

  if (bzla_sort_is_bool(d_bzla, bzla_node_get_sort_id(node))
      && d_internal->d_prop_map.find(node) != d_internal->d_prop_map.end())
  {
    return d_internal->d_prop_map.at(node).getNode();
  }

  if (bzla_node_is_rm(d_bzla, node)
      && d_internal->d_rm_map.find(node) != d_internal->d_rm_map.end())
  {
    return d_internal->d_rm_map.at(node).getNode();
  }

  if (d_internal->d_unpacked_float_map.find(node)
      != d_internal->d_unpacked_float_map.end())
  {
    d_internal->d_packed_float_map.emplace(
        node,
        symfpu::pack(bzla_node_get_sort_id(node),
                     d_internal->d_unpacked_float_map.at(node)));
    return d_internal->d_packed_float_map.at(node).getNode();
  }

  return word_blast(node);
}

void
WordBlaster::get_introduced_ufs(std::vector<BzlaNode *> &ufs)
{
  for (const auto &p : d_min_max_uf_map)
  {
    ufs.push_back(p.second);
  }
  for (const auto &p : d_sbv_ubv_uf_map)
  {
    ufs.push_back(p.second);
  }
}

void
WordBlaster::add_additional_assertions()
{
  for (BzlaNode *node : d_additional_assertions)
  {
    bzla_assert_exp(d_bzla, node);
    bzla_node_release(d_bzla, node);
  }
  d_additional_assertions.clear();
}

WordBlaster *
WordBlaster::clone(Bzla *cbzla, BzlaNodeMap *exp_map)
{
  BzlaNode *exp, *cexp;
  WordBlaster *res = new WordBlaster(cbzla);

  for (const auto &p : d_min_max_uf_map)
  {
    BzlaSortId s = p.first;
    exp          = p.second;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_min_max_uf_map.find(s) == res->d_min_max_uf_map.end());
    res->d_min_max_uf_map.emplace(s, cexp);
  }
  for (const auto &p : d_sbv_ubv_uf_map)
  {
    exp = p.second;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_sbv_ubv_uf_map.find(p.first) == res->d_sbv_ubv_uf_map.end());
    res->d_sbv_ubv_uf_map.emplace(p.first, cexp);
  }
  for (const auto &p : d_internal->d_rm_map)
  {
    exp = p.first;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_internal->d_rm_map.find(cexp)
           == res->d_internal->d_rm_map.end());

    BzlaNode *sexp  = d_internal->d_rm_map.at(exp).getNode();
    BzlaNode *scexp = bzla_nodemap_mapped(exp_map, sexp);
    assert(scexp);
    res->d_internal->d_rm_map.emplace(cexp, SymFpuSymRM(scexp));
  }
  for (const auto &p : d_internal->d_prop_map)
  {
    exp = p.first;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_internal->d_prop_map.find(cexp)
           == res->d_internal->d_prop_map.end());

    BzlaNode *sexp  = d_internal->d_prop_map.at(exp).getNode();
    BzlaNode *scexp = bzla_nodemap_mapped(exp_map, sexp);
    assert(scexp);
    res->d_internal->d_prop_map.emplace(cexp, SymFpuSymProp(scexp));
  }
  for (const auto &p : d_internal->d_sbv_map)
  {
    exp = p.first;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_internal->d_sbv_map.find(cexp)
           == res->d_internal->d_sbv_map.end());

    BzlaNode *sexp  = d_internal->d_sbv_map.at(exp).getNode();
    BzlaNode *scexp = bzla_nodemap_mapped(exp_map, sexp);
    assert(scexp);
    res->d_internal->d_sbv_map.emplace(cexp, SymFpuSymBV<true>(scexp));
  }
  for (const auto &p : d_internal->d_ubv_map)
  {
    exp = p.first;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_internal->d_ubv_map.find(cexp)
           == res->d_internal->d_ubv_map.end());

    BzlaNode *sexp  = d_internal->d_ubv_map.at(exp).getNode();
    BzlaNode *scexp = bzla_nodemap_mapped(exp_map, sexp);
    assert(scexp);
    res->d_internal->d_ubv_map.emplace(cexp, SymFpuSymBV<false>(scexp));
  }
  for (const auto &p : d_internal->d_unpacked_float_map)
  {
    exp = p.first;
    assert(bzla_node_is_regular(exp));
    cexp = bzla_nodemap_mapped(exp_map, exp);
    assert(cexp);
    assert(res->d_internal->d_unpacked_float_map.find(cexp)
           == res->d_internal->d_unpacked_float_map.end());

    BzlaNode *nan = p.second.getNaN().getNode();
    assert(nan);
    BzlaNode *cnan = bzla_nodemap_mapped(exp_map, nan);
    assert(cnan);

    BzlaNode *inf = p.second.getInf().getNode();
    assert(inf);
    BzlaNode *cinf = bzla_nodemap_mapped(exp_map, inf);
    assert(cinf);

    BzlaNode *zero = p.second.getZero().getNode();
    assert(zero);
    BzlaNode *czero = bzla_nodemap_mapped(exp_map, zero);
    assert(czero);

    BzlaNode *sign = p.second.getSign().getNode();
    assert(sign);
    BzlaNode *csign = bzla_nodemap_mapped(exp_map, sign);
    assert(csign);

    BzlaNode *expo = p.second.getExponent().getNode();
    assert(expo);
    BzlaNode *cexpo = bzla_nodemap_mapped(exp_map, expo);
    assert(cexpo);

    BzlaNode *sig = p.second.getSignificand().getNode();
    assert(sig);
    BzlaNode *csig = bzla_nodemap_mapped(exp_map, sig);
    assert(csig);

    res->d_internal->d_unpacked_float_map.emplace(
        cexp,
        SymUnpackedFloat(SymFpuSymProp(cnan),
                         SymFpuSymProp(cinf),
                         SymFpuSymProp(czero),
                         SymFpuSymProp(csign),
                         SymFpuSymBV<true>(cexpo),
                         SymFpuSymBV<false>(csig)));
  }
  for (BzlaNode *node : d_additional_assertions)
  {
    BzlaNode *real_node = bzla_node_real_addr(node);
    cexp                = bzla_nodemap_mapped(exp_map, real_node);
    assert(cexp);
    res->d_additional_assertions.push_back(bzla_node_cond_invert(node, cexp));
  }
  return res;
}

/* --- WordBlaster private -------------------------------------------------- */

BzlaNode *
WordBlaster::min_max_uf(BzlaNode *node)
{
  assert(bzla_node_is_regular(node));

  BzlaSortId sort_fp = bzla_node_get_sort_id(node);

  if (d_min_max_uf_map.find(sort_fp) != d_min_max_uf_map.end())
    return d_min_max_uf_map.at(sort_fp);

  uint32_t arity      = node->arity;
  uint32_t bw         = bzla_sort_fp_get_bv_width(d_bzla, sort_fp);
  BzlaSortId sort_bv1 = bzla_sort_bv(d_bzla, 1);
  BzlaSortId sort_bv  = bzla_sort_bv(d_bzla, bw);
  BzlaSortId sorts[2];
  for (uint32_t i = 0; i < arity; ++i) sorts[i] = sort_bv;
  BzlaSortId sort_domain = bzla_sort_tuple(d_bzla, sorts, arity);
  BzlaSortId sort_fun    = bzla_sort_fun(d_bzla, sort_domain, sort_bv1);
  std::stringstream ss;
  ss << (bzla_node_is_fp_min(node) ? "_fp_min_uf_" : "_fp_max_uf_")
     << bzla_node_get_id(node) << "_";
  d_min_max_uf_map.emplace(bzla_sort_copy(d_bzla, sort_fp),
                           bzla_exp_uf(d_bzla, sort_fun, ss.str().c_str()));
  bzla_sort_release(d_bzla, sort_fun);
  bzla_sort_release(d_bzla, sort_domain);
  bzla_sort_release(d_bzla, sort_bv);
  bzla_sort_release(d_bzla, sort_bv1);
  return d_min_max_uf_map.at(sort_fp);
}

BzlaNode *
WordBlaster::sbv_ubv_uf(BzlaNode *node)
{
  assert(bzla_node_is_regular(node));
  assert(bzla_node_is_rm(d_bzla, node->e[0]));
  assert(bzla_node_is_fp(d_bzla, node->e[1]));

  BzlaSortId sort_bv = bzla_node_get_sort_id(node);
  BzlaSortId sort_fp = bzla_node_get_sort_id(node->e[1]);
  std::pair<BzlaSortId, BzlaSortId> p(sort_fp, sort_bv);

  if (d_sbv_ubv_uf_map.find(p) != d_sbv_ubv_uf_map.end())
    return d_sbv_ubv_uf_map.at(p);

  BzlaSortId sorts[2]    = {bzla_node_get_sort_id(node->e[0]), sort_fp};
  BzlaSortId sort_domain = bzla_sort_tuple(d_bzla, sorts, 2);
  BzlaSortId sort_fun    = bzla_sort_fun(d_bzla, sort_domain, sort_bv);

  std::stringstream ss;
  ss << (bzla_node_is_fp_to_sbv(node) ? "_fp_sbv_uf_" : "_fp_ubv_uf_")
     << bzla_node_get_id(node) << "_";
  (void) bzla_sort_copy(d_bzla, sort_fp);
  (void) bzla_sort_copy(d_bzla, sort_bv);
  d_sbv_ubv_uf_map.emplace(p, bzla_exp_uf(d_bzla, sort_fun, ss.str().c_str()));
  bzla_sort_release(d_bzla, sort_fun);
  bzla_sort_release(d_bzla, sort_domain);
  return d_sbv_ubv_uf_map.at(p);
}

/* -------------------------------------------------------------------------- */
}  // namespace fp
}  // namespace bzla