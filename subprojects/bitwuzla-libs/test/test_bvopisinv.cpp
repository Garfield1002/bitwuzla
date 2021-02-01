#include "bitvector_op.h"
#include "gmprandstate.h"
#include "rng.h"
#include "test.h"

namespace bzlals {
namespace test {

class TestBvOpIsInv : public TestBvDomainCommon
{
 protected:
  void SetUp() override
  {
    TestBvDomainCommon::SetUp();
    gen_values(TEST_BW, d_values);
    gen_xvalues(TEST_BW, d_xvalues);
    d_rng.reset(new RNG(1234));
  }

  bool check_sat_binary(Kind kind,
                        const BitVectorDomain& x,
                        const BitVector& t,
                        const BitVector& s,
                        uint32_t pos_x);
  bool check_sat_ite(const BitVectorDomain& x,
                     const BitVector& t,
                     const BitVector& s0,
                     const BitVector& s1,
                     uint32_t pos_x);

  template <class T>
  void test_binary(Kind kind, uint32_t pos_x, bool const_bits);
  void test_ite(uint32_t pos_x, bool const_bits);

  static constexpr uint32_t TEST_BW = 4;
  std::vector<std::string> d_values;
  std::vector<std::string> d_xvalues;
  std::unique_ptr<RNG> d_rng;
};

bool
TestBvOpIsInv::check_sat_binary(Kind kind,
                                const BitVectorDomain& x,
                                const BitVector& t,
                                const BitVector& s,
                                uint32_t pos_x)
{
  BitVectorDomainGenerator gen(x);
  do
  {
    BitVector val = gen.has_next() ? gen.next() : x.lo();
    BitVector res;
    switch (kind)
    {
      case ADD: res = pos_x ? s.bvadd(val) : val.bvadd(s); break;
      case AND: res = pos_x ? s.bvand(val) : val.bvand(s); break;
      case ASHR: res = pos_x ? s.bvashr(val) : val.bvashr(s); break;
      case CONCAT: res = pos_x ? s.bvconcat(val) : val.bvconcat(s); break;
      case EQ: res = pos_x ? s.bveq(val) : val.bveq(s); break;
      case IMPLIES: res = pos_x ? s.bvimplies(val) : val.bvimplies(s); break;
      case MUL: res = pos_x ? s.bvmul(val) : val.bvmul(s); break;
      case NAND: res = pos_x ? s.bvnand(val) : val.bvnand(s); break;
      case NE: res = pos_x ? s.bvne(val) : val.bvne(s); break;
      case NOR: res = pos_x ? s.bvnor(val) : val.bvnor(s); break;
      case OR: res = pos_x ? s.bvor(val) : val.bvor(s); break;
      case SDIV: res = pos_x ? s.bvsdiv(val) : val.bvsdiv(s); break;
      case SGT: res = pos_x ? s.bvsgt(val) : val.bvsgt(s); break;
      case SGE: res = pos_x ? s.bvsge(val) : val.bvsge(s); break;
      case SHL: res = pos_x ? s.bvshl(val) : val.bvshl(s); break;
      case SHR: res = pos_x ? s.bvshr(val) : val.bvshr(s); break;
      case SLT: res = pos_x ? s.bvslt(val) : val.bvslt(s); break;
      case SLE: res = pos_x ? s.bvsle(val) : val.bvsle(s); break;
      case SREM: res = pos_x ? s.bvsrem(val) : val.bvsrem(s); break;
      case SUB: res = pos_x ? s.bvsub(val) : val.bvsub(s); break;
      case UDIV: res = pos_x ? s.bvudiv(val) : val.bvudiv(s); break;
      case UGT: res = pos_x ? s.bvugt(val) : val.bvugt(s); break;
      case UGE: res = pos_x ? s.bvuge(val) : val.bvuge(s); break;
      case ULT: res = pos_x ? s.bvult(val) : val.bvult(s); break;
      case ULE: res = pos_x ? s.bvule(val) : val.bvule(s); break;
      case UREM: res = pos_x ? s.bvurem(val) : val.bvurem(s); break;
      case XNOR: res = pos_x ? s.bvxnor(val) : val.bvxnor(s); break;
      case XOR: res = pos_x ? s.bvxor(val) : val.bvxor(s); break;
      default: assert(false);
    }
    if (t.compare(res) == 0) return true;
  } while (gen.has_next());
  return false;
}

bool
TestBvOpIsInv::check_sat_ite(const BitVectorDomain& x,
                             const BitVector& t,
                             const BitVector& s0,
                             const BitVector& s1,
                             uint32_t pos_x)
{
  BitVectorDomainGenerator gen(x);
  do
  {
    BitVector val = gen.has_next() ? gen.next() : x.lo();
    BitVector res;
    if (pos_x == 0)
    {
      res = BitVector::bvite(val, s0, s1);
    }
    else if (pos_x == 1)
    {
      if (s0.is_false()) return false;
      res = BitVector::bvite(s0, val, s1);
    }
    else
    {
      if (s0.is_true()) return false;
      res = BitVector::bvite(s0, s1, val);
    }
    if (t.compare(res) == 0) return true;
  } while (gen.has_next());
  return false;
}

template <class T>
void
TestBvOpIsInv::test_binary(Kind kind, uint32_t pos_x, bool const_bits)
{
  std::vector<std::string> x_values;

  uint32_t bw_x = TEST_BW;
  uint32_t bw_s = TEST_BW;
  uint32_t bw_t = TEST_BW;

  if (const_bits)
  {
    x_values = d_xvalues;
  }
  else
  {
    /* x is unconstrained (no const bits) */
    x_values.push_back(std::string(bw_x, 'x'));
  }

  if (kind == ULT || kind == SLT || kind == EQ)
  {
    bw_t = 1;
  }
  else if (kind == CONCAT)
  {
    bw_s = 2; /* decrease number of tests for concat */
    bw_t = bw_s + bw_x;
  }

  uint32_t nval_s = 1 << bw_s;
  uint32_t nval_t = 1 << bw_t;
  for (const std::string& x_value : x_values)
  {
    BitVectorDomain x(x_value);
    for (uint32_t i = 0; i < nval_s; i++)
    {
      /* Assignment of the other operand. */
      BitVector s(bw_s, i);
      for (uint32_t j = 0; j < nval_t; j++)
      {
        /* Target value of the operation (op). */
        BitVector t(bw_t, j);
        /* For this test, we don't care about the current assignment of x, thus
         * we initialize it with 0. */
        BitVectorOp* op_x =
            new T(d_rng.get(), BitVector::mk_zero(bw_x), x, nullptr, nullptr);
        /* For this test, we don't care about the domain of s, thus we
         * initialize it with an unconstrained domain. */
        BitVectorOp* op_s =
            new T(d_rng.get(), s, BitVectorDomain(bw_s), nullptr, nullptr);
        /* For this test, we don't care about current assignment and domain of
         * the op, thus we initialize them with 0 and 'x..x', respectively. */
        T op(d_rng.get(),
             bw_t,
             pos_x == 0 ? op_x : op_s,
             pos_x == 1 ? op_x : op_s);

        bool res    = op.is_invertible(t, pos_x);
        bool status = check_sat_binary(kind, x, t, s, pos_x);
        if (res != status)
        {
          std::cout << "pos_x: " << pos_x << std::endl;
          std::cout << "t: " << t.to_string() << std::endl;
          std::cout << "x: " << x_value << std::endl;
          std::cout << "s: " << s.to_string() << std::endl;
        }
        ASSERT_EQ(res, status);

        delete op[pos_x];
        delete op[1 - pos_x];
      }
    }
  }
}

void
TestBvOpIsInv::test_ite(uint32_t pos_x, bool const_bits)
{
  std::vector<std::string> x_values;
  uint32_t bw_s0, bw_s1, bw = TEST_BW;
  uint32_t n_vals, n_vals_s0, n_vals_s1;
  uint32_t pos_s0 = pos_x == 0 ? 1 : 0;
  uint32_t pos_s1 = pos_x == 2 ? 1 : 2;

  if (pos_x)
  {
    bw_s0 = 1;
    bw_s1 = bw;
    if (const_bits)
    {
      x_values = d_xvalues;
    }
    else
    {
      x_values.push_back(std::string(bw, 'x'));
    }
  }
  else
  {
    bw_s0 = bw;
    bw_s1 = bw;
    if (const_bits)
    {
      x_values.push_back("x");
      x_values.push_back("0");
      x_values.push_back("1");
    }
    else
    {
      x_values.push_back("x");
    }
  }
  n_vals    = 1 << bw;
  n_vals_s0 = 1 << bw_s0;
  n_vals_s1 = 1 << bw_s1;

  for (const std::string& x_value : x_values)
  {
    BitVectorDomain x(x_value);
    for (uint32_t i = 0; i < n_vals_s0; i++)
    {
      BitVector s0(bw_s0, i);
      for (uint32_t j = 0; j < n_vals_s1; j++)
      {
        BitVector s1(bw_s1, j);
        for (uint32_t k = 0; k < n_vals; k++)
        {
          BitVector t(bw, k);

          /* For this test, we don't care about the current assignment of x,
           * thus we initialize it with 0. */
          BitVectorOp* op_x = new BitVectorIte(d_rng.get(),
                                               BitVector::mk_zero(bw),
                                               x,
                                               nullptr,
                                               nullptr,
                                               nullptr);
          /* For this test, we don't care about the domain of 0, thus we
           * initialize it with an unconstrained domain. */
          BitVectorOp* op_s0 = new BitVectorIte(d_rng.get(),
                                                s0,
                                                BitVectorDomain(bw_s0),
                                                nullptr,
                                                nullptr,
                                                nullptr);
          BitVectorOp* op_s1 = new BitVectorIte(d_rng.get(),
                                                s1,
                                                BitVectorDomain(bw_s0),
                                                nullptr,
                                                nullptr,
                                                nullptr);
          /* For this test, we don't care about current assignment and domain of
           * the op, thus we initialize them with 0 and 'x..x', respectively. */
          BitVectorIte op(d_rng.get(),
                          bw,
                          pos_x == 0 ? op_x : op_s0,
                          pos_x == 1 ? op_x : (pos_x == 2 ? op_s1 : op_s0),
                          pos_x == 2 ? op_x : op_s1);

          bool res    = op.is_invertible(t, pos_x);
          bool status = check_sat_ite(x, t, s0, s1, pos_x);
          if (res != status)
          {
            std::cout << "pos_x: " << pos_x << std::endl;
            std::cout << "t: " << t.to_string() << std::endl;
            std::cout << "x: " << x_value << std::endl;
            std::cout << "s0: " << s0.to_string() << std::endl;
            std::cout << "s1: " << s1.to_string() << std::endl;
          }
          ASSERT_EQ(res, status);

          delete op[pos_x];
          delete op[pos_s0];
          delete op[pos_s1];
        }
      }
    }
  }
}

TEST_F(TestBvOpIsInv, add)
{
  test_binary<BitVectorAdd>(ADD, 0, false);
  test_binary<BitVectorAdd>(ADD, 1, false);
  test_binary<BitVectorAdd>(ADD, 0, true);
  test_binary<BitVectorAdd>(ADD, 1, true);
}

TEST_F(TestBvOpIsInv, and)
{
  test_binary<BitVectorAnd>(AND, 0, false);
  test_binary<BitVectorAnd>(AND, 1, false);
  test_binary<BitVectorAnd>(AND, 0, true);
  test_binary<BitVectorAnd>(AND, 1, true);
}

TEST_F(TestBvOpIsInv, concat)
{
  test_binary<BitVectorConcat>(CONCAT, 0, false);
  test_binary<BitVectorConcat>(CONCAT, 1, false);
  test_binary<BitVectorConcat>(CONCAT, 0, true);
  test_binary<BitVectorConcat>(CONCAT, 1, true);
}

TEST_F(TestBvOpIsInv, eq)
{
  test_binary<BitVectorEq>(EQ, 0, false);
  test_binary<BitVectorEq>(EQ, 1, false);
  test_binary<BitVectorEq>(EQ, 0, true);
  test_binary<BitVectorEq>(EQ, 1, true);
}

TEST_F(TestBvOpIsInv, mul)
{
  test_binary<BitVectorMul>(MUL, 0, false);
  test_binary<BitVectorMul>(MUL, 1, false);
  test_binary<BitVectorMul>(MUL, 0, true);
  test_binary<BitVectorMul>(MUL, 1, true);
}

TEST_F(TestBvOpIsInv, shl)
{
  test_binary<BitVectorShl>(SHL, 0, false);
  test_binary<BitVectorShl>(SHL, 1, false);
  test_binary<BitVectorShl>(SHL, 0, true);
  test_binary<BitVectorShl>(SHL, 1, true);
}

TEST_F(TestBvOpIsInv, shr)
{
  test_binary<BitVectorShr>(SHR, 0, false);
  test_binary<BitVectorShr>(SHR, 1, false);
  test_binary<BitVectorShr>(SHR, 0, true);
  test_binary<BitVectorShr>(SHR, 1, true);
}

TEST_F(TestBvOpIsInv, ashr)
{
  test_binary<BitVectorAshr>(ASHR, 0, false);
  test_binary<BitVectorAshr>(ASHR, 1, false);
  test_binary<BitVectorAshr>(ASHR, 0, true);
  test_binary<BitVectorAshr>(ASHR, 1, true);
}

TEST_F(TestBvOpIsInv, udiv)
{
  test_binary<BitVectorUdiv>(UDIV, 0, false);
  test_binary<BitVectorUdiv>(UDIV, 1, false);
  test_binary<BitVectorUdiv>(UDIV, 0, true);
  test_binary<BitVectorUdiv>(UDIV, 1, true);
}

TEST_F(TestBvOpIsInv, ult)
{
  test_binary<BitVectorUlt>(ULT, 0, false);
  test_binary<BitVectorUlt>(ULT, 1, false);
  test_binary<BitVectorUlt>(ULT, 0, true);
  test_binary<BitVectorUlt>(ULT, 1, true);
}

TEST_F(TestBvOpIsInv, slt)
{
  test_binary<BitVectorSlt>(SLT, 0, false);
  test_binary<BitVectorSlt>(SLT, 1, false);
  test_binary<BitVectorSlt>(SLT, 0, true);
  test_binary<BitVectorSlt>(SLT, 1, true);
}

TEST_F(TestBvOpIsInv, urem)
{
  test_binary<BitVectorUrem>(UREM, 0, false);
  test_binary<BitVectorUrem>(UREM, 1, false);
  test_binary<BitVectorUrem>(UREM, 0, true);
  test_binary<BitVectorUrem>(UREM, 1, true);
}

TEST_F(TestBvOpIsInv, ite)
{
  test_ite(0, false);
  test_ite(1, false);
  test_ite(2, false);
  test_ite(0, true);
  test_ite(1, true);
  test_ite(2, true);
}
}  // namespace test
}  // namespace bzlals