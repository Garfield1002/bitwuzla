#include <bitset>
#include <iostream>

#include "gmprandstate.h"
#include "rng.h"
#include "test.h"

namespace bzlals {
namespace test {

/* -------------------------------------------------------------------------- */

class TestBitVector : public TestCommon
{
 protected:
  enum Kind
  {
    ADD,
    AND,
    ASHR,
    DEC,
    EQ,
    IMPLIES,
    ITE,
    INC,
    MUL,
    NAND,
    NE,
    NEG,
    NOR,
    NOT,
    OR,
    REDAND,
    REDOR,
    SDIV,
    SEXT,
    SGT,
    SGE,
    SHL,
    SHR,
    SLT,
    SLE,
    SREM,
    SUB,
    UDIV,
    UGT,
    UGE,
    ULT,
    ULE,
    UREM,
    XNOR,
    XOR,
    ZEXT,
  };

  enum BvFunKind
  {
    DEFAULT,
    INPLACE_CHAINABLE,
    INPLACE_NOT_CHAINABLE,
  };

  static constexpr uint32_t N_TESTS        = 100000;
  static constexpr uint32_t N_MODINV_TESTS = 100000;
  void SetUp() override { d_rng.reset(new RNG(1234)); }

  static uint64_t _add(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _and(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ashr(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _dec(uint64_t x, uint32_t size);
  static uint64_t _eq(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _implies(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ite(uint64_t c, uint64_t t, uint64_t e, uint32_t size);
  static uint64_t _inc(uint64_t x, uint32_t size);
  static uint64_t _mul(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _nand(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ne(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _neg(uint64_t x, uint32_t size);
  static uint64_t _nor(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _not(uint64_t x, uint32_t size);
  static uint64_t _or(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _redand(uint64_t x, uint32_t size);
  static uint64_t _redor(uint64_t x, uint32_t size);
  static int64_t _sdiv(int64_t x, int64_t y, uint32_t size);
  static int64_t _sgt(int64_t x, int64_t y, uint32_t size);
  static int64_t _sge(int64_t x, int64_t y, uint32_t size);
  static uint64_t _shl(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _shr(uint64_t x, uint64_t y, uint32_t size);
  static int64_t _slt(int64_t x, int64_t y, uint32_t size);
  static int64_t _sle(int64_t x, int64_t y, uint32_t size);
  static int64_t _srem(int64_t x, int64_t y, uint32_t size);
  static uint64_t _sub(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _udiv(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ugt(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _uge(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ult(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _ule(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _urem(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _xnor(uint64_t x, uint64_t y, uint32_t size);
  static uint64_t _xor(uint64_t x, uint64_t y, uint32_t size);

  BitVector mk_ones(uint32_t size);
  BitVector mk_min_signed(uint32_t size);
  BitVector mk_max_signed(uint32_t size);
  void test_ctor_random_bit_range(uint32_t size);
  void test_count(uint32_t size, bool leading, bool zeros);
  void test_count_aux(const std::string& val, bool leading, bool zeros);
  void test_unary(BvFunKind fun_kind, Kind kind, uint32_t size);
  void test_binary(BvFunKind fun_kind, Kind kind, uint32_t size);
  void test_binary_signed(BvFunKind fun_kind, Kind kind, uint32_t size);
  void test_concat(BvFunKind fun_kind, uint32_t size);
  void test_extend(BvFunKind fun_kind, Kind kind, uint32_t size);
  void test_extract(BvFunKind fun_kind, uint32_t size);
  void test_is_uadd_overflow_aux(uint32_t size,
                                 uint64_t a1,
                                 uint64_t a2,
                                 bool expected);
  void test_is_uadd_overflow(uint32_t size);
  void test_is_umul_overflow_aux(uint32_t size,
                                 uint64_t a1,
                                 uint64_t a2,
                                 bool expected);
  void test_is_umul_overflow(uint32_t size);
  void test_ite(BvFunKind fun_kind, uint32_t size);
  void test_modinv(BvFunKind fun_kind, uint32_t size);
  void test_shift_aux(BvFunKind fun_kind,
                      Kind kind,
                      const std::string& to_shift,
                      const std::string& shift,
                      const std::string& expected,
                      bool shift_by_int);
  void test_shift(BvFunKind fun_kind, Kind kind, bool shift_by_int);
  void test_udivurem(uint32_t size);
  std::unique_ptr<RNG> d_rng;
};

uint64_t
TestBitVector::_not(uint64_t x, uint32_t size)
{
  return ~x % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_neg(uint64_t x, uint32_t size)
{
  return -x % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_redand(uint64_t x, uint32_t size)
{
  uint64_t a = UINT64_MAX << size;
  return (x + a) == UINT64_MAX;
}

uint64_t
TestBitVector::_redor(uint64_t x, uint32_t size)
{
  (void) size;
  return x != 0;
}

uint64_t
TestBitVector::_inc(uint64_t x, uint32_t size)
{
  return (x + 1) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_dec(uint64_t x, uint32_t size)
{
  return (x - 1) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_add(uint64_t x, uint64_t y, uint32_t size)
{
  return (x + y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_sub(uint64_t x, uint64_t y, uint32_t size)
{
  return (x - y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_and(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x & y;
}

uint64_t
TestBitVector::_nand(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  uint32_t shift = 64 - size;
  return (((~(x & y)) << shift) >> shift);
}

uint64_t
TestBitVector::_or(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x | y;
}

uint64_t
TestBitVector::_nor(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  uint32_t shift = 64 - size;
  return ((~(x | y)) << shift) >> shift;
}

uint64_t
TestBitVector::_xnor(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  uint32_t shift = 64 - size;
  return ((~(x ^ y)) << shift) >> shift;
}

uint64_t
TestBitVector::_implies(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size == 1);
  (void) size;
  return ((~x | y) << 63) >> 63;
}

uint64_t
TestBitVector::_xor(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x ^ y;
}

uint64_t
TestBitVector::_eq(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x == y;
}

uint64_t
TestBitVector::_ne(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x != y;
}

uint64_t
TestBitVector::_ult(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x < y;
}

uint64_t
TestBitVector::_ule(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x <= y;
}

uint64_t
TestBitVector::_ugt(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x > y;
}

uint64_t
TestBitVector::_uge(uint64_t x, uint64_t y, uint32_t size)
{
  (void) size;
  return x >= y;
}

int64_t
TestBitVector::_slt(int64_t x, int64_t y, uint32_t size)
{
  (void) size;
  return x < y;
}

int64_t
TestBitVector::_sle(int64_t x, int64_t y, uint32_t size)
{
  (void) size;
  return x <= y;
}

int64_t
TestBitVector::_sgt(int64_t x, int64_t y, uint32_t size)
{
  (void) size;
  return x > y;
}

int64_t
TestBitVector::_sge(int64_t x, int64_t y, uint32_t size)
{
  (void) size;
  return x >= y;
}

uint64_t
TestBitVector::_shl(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  if (y >= size) return 0;
  return (x << y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_shr(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  if (y >= size) return 0;
  return (x >> y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_ashr(uint64_t x, uint64_t y, uint32_t size)
{
  assert(size <= 64);
  uint64_t max = pow(2, size);
  if ((x >> (size - 1)) & 1)
  {
    if (y > size) return ~0 % max;
    return ~((~x % max) >> y) % max;
  }
  if (y > size) return 0;
  return (x >> y) % max;
}

uint64_t
TestBitVector::_mul(uint64_t x, uint64_t y, uint32_t size)
{
  return (x * y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_udiv(uint64_t x, uint64_t y, uint32_t size)
{
  if (y == 0) return UINT64_MAX % (uint64_t) pow(2, size);
  return (x / y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_urem(uint64_t x, uint64_t y, uint32_t size)
{
  if (y == 0) return x;
  return (x % y) % (uint64_t) pow(2, size);
}

int64_t
TestBitVector::_sdiv(int64_t x, int64_t y, uint32_t size)
{
  if (y == 0)
  {
    return x < 0 ? 1 : UINT64_MAX % (uint64_t) pow(2, size);
  }
  return (x / y) % (uint64_t) pow(2, size);
}

int64_t
TestBitVector::_srem(int64_t x, int64_t y, uint32_t size)
{
  if (y == 0) return x % (uint64_t) pow(2, size);
  return (x % y) % (uint64_t) pow(2, size);
}

uint64_t
TestBitVector::_ite(uint64_t c, uint64_t t, uint64_t e, uint32_t size)
{
  (void) size;
  return c ? t : e;
}

BitVector
TestBitVector::mk_ones(uint32_t size)
{
  if (size <= 64)
  {
    return BitVector(size, UINT64_MAX);
  }
  BitVector r(64, UINT64_MAX);
  BitVector l(size - 64, UINT64_MAX);
  return l.bvconcat(r);
}

BitVector
TestBitVector::mk_min_signed(uint32_t size)
{
  if (size <= 64)
  {
    return BitVector(size, ((uint64_t) 1) << (size - 1));
  }
  BitVector r(64, 0);
  BitVector l(size - 64, ((uint64_t) 1) << (size - 1 - 64));
  return l.bvconcat(r);
}

BitVector
TestBitVector::mk_max_signed(uint32_t size)
{
  if (size <= 64)
  {
    return BitVector(size, (((uint64_t) 1) << (size - 1)) - 1);
  }
  BitVector r(64, UINT64_MAX);
  BitVector l(size - 64, (((uint64_t) 1) << (size - 1 - 64)) - 1);
  return l.bvconcat(r);
}

void
TestBitVector::test_ctor_random_bit_range(uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    uint32_t up, lo;
    lo = d_rng->pick<uint32_t>(0, size - 1);
    up = lo == size - 1 ? size - 1 : d_rng->pick<uint32_t>(lo + 1, size - 1);
    BitVector bv1(size, *d_rng, up, lo);
    BitVector bv2(size, *d_rng, up, lo);
    BitVector bv3(size, *d_rng, up, lo);
    for (uint32_t j = lo; j <= up; ++j)
    {
      if (bv1.get_bit(j) != bv2.get_bit(j) || bv1.get_bit(j) != bv3.get_bit(j)
          || bv2.get_bit(j) != bv3.get_bit(j))
      {
        break;
      }
    }
    for (uint32_t j = 0; j < lo; ++j)
    {
      assert(bv1.get_bit(j) == 0);
      assert(bv2.get_bit(j) == 0);
      assert(bv3.get_bit(j) == 0);
    }
    for (uint32_t j = up + 1; j < size; j++)
    {
      assert(bv1.get_bit(j) == 0);
      assert(bv2.get_bit(j) == 0);
      assert(bv3.get_bit(j) == 0);
    }
  }
}

void
TestBitVector::test_count_aux(const std::string& val, bool leading, bool zeros)
{
  uint32_t size     = val.size();
  uint32_t expected = 0;
  char c            = zeros ? '0' : '1';
  BitVector bv(size, val);
  if (leading)
  {
    for (expected = 0; expected < size && val[expected] == c; ++expected)
      ;
    if (zeros)
    {
      ASSERT_EQ(bv.count_leading_zeros(), expected);
    }
    else
    {
      ASSERT_EQ(bv.count_leading_ones(), expected);
    }
  }
  else
  {
    for (expected = 0; expected < size && val[size - 1 - expected] == c;
         ++expected)
      ;
    assert(zeros);
    ASSERT_EQ(bv.count_trailing_zeros(), expected);
  }
}

void
TestBitVector::test_count(uint32_t size, bool leading, bool zeros)
{
  if (size == 8)
  {
    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      ss << std::bitset<8>(i).to_string();
      test_count_aux(ss.str(), leading, zeros);
    }
  }
  else
  {
    // concat 8-bit value with 0s to create value for bv
    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << v << std::string(size - 8, '0');
      test_count_aux(ss.str(), leading, zeros);
    }

    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << std::string(size - 8, '0') << v;
      test_count_aux(ss.str(), leading, zeros);
    }

    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << v << std::string(size - 16, '0') << v;
      test_count_aux(ss.str(), leading, zeros);
    }

    // concat 8-bit values with 1s to create value for bv
    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << v << std::string(size - 8, '1');
      test_count_aux(ss.str(), leading, zeros);
    }

    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << std::string(size - 8, '1') << v;
      test_count_aux(ss.str(), leading, zeros);
    }

    for (uint64_t i = 0; i < (1u << 8); ++i)
    {
      std::stringstream ss;
      std::string v = std::bitset<8>(i).to_string();
      ss << v << std::string(size - 16, '1') << v;
      test_count_aux(ss.str(), leading, zeros);
    }
  }
}

void
TestBitVector::test_extend(BvFunKind fun_kind, Kind kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    uint32_t n = d_rng->pick<uint32_t>(0, size - 1);
    BitVector bv(size - n, *d_rng);
    BitVector res(size);
    char c = 0;

    switch (kind)
    {
      case ZEXT:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          // TODO
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvzext(bv, n);
        }
        else
        {
          res = bv.bvzext(n);
        }
        c = '0';
        break;
      case SEXT:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          // TODO
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvsext(bv, n);
        }
        else
        {
          res = bv.bvsext(n);
        }
        c = bv.get_msb() ? '1' : '0';
        break;

      default: assert(false);
    }
    ASSERT_EQ(bv.size() + n, res.size());
    std::string res_str = res.to_string();
    std::string bv_str  = bv.to_string();
    uint32_t len        = size - n;
    ASSERT_EQ(bv_str.compare(0, len, res_str, n, len), 0);
    ASSERT_EQ(std::string(n, c).compare(0, n, res_str, 0, n), 0);
  }
}

void
TestBitVector::test_is_uadd_overflow_aux(uint32_t size,
                                         uint64_t a1,
                                         uint64_t a2,
                                         bool expected)
{
  BitVector bv1(size, a1);
  BitVector bv2(size, a2);
  ASSERT_EQ(bv1.is_uadd_overflow(bv2), expected);
  ASSERT_DEATH(bv1.is_uadd_overflow(BitVector(size + 1, *d_rng)),
               "d_size == bv.d_size");
}

void
TestBitVector::test_is_uadd_overflow(uint32_t size)
{
  switch (size)
  {
    case 1:
      test_is_uadd_overflow_aux(size, 0, 0, false);
      test_is_uadd_overflow_aux(size, 0, 1, false);
      test_is_uadd_overflow_aux(size, 1, 1, true);
      break;
    case 7:
      test_is_uadd_overflow_aux(size, 3, 6, false);
      test_is_uadd_overflow_aux(size, 126, 2, true);
      break;
    case 31:
      test_is_uadd_overflow_aux(size, 15, 78, false);
      test_is_uadd_overflow_aux(size, 2147483647, 2147483650, true);
      break;
    case 33:
      test_is_uadd_overflow_aux(size, 15, 78, false);
      test_is_uadd_overflow_aux(size, 4294967295, 4294967530, true);
      break;
    default: assert(false);
  }
}

void
TestBitVector::test_is_umul_overflow_aux(uint32_t size,
                                         uint64_t a1,
                                         uint64_t a2,
                                         bool expected)
{
  BitVector bv1(size, a1);
  BitVector bv2(size, a2);
  ASSERT_EQ(bv1.is_umul_overflow(bv2), expected);
  ASSERT_DEATH(bv1.is_umul_overflow(BitVector(size + 1, *d_rng)),
               "d_size == bv.d_size");
}

void
TestBitVector::test_is_umul_overflow(uint32_t size)
{
  switch (size)
  {
    case 1:
      test_is_umul_overflow_aux(size, 0, 0, false);
      test_is_umul_overflow_aux(size, 0, 1, false);
      test_is_umul_overflow_aux(size, 1, 1, false);
      break;
    case 7:
      test_is_umul_overflow_aux(size, 3, 6, false);
      test_is_umul_overflow_aux(size, 124, 2, true);
      break;
    case 31:
      test_is_umul_overflow_aux(size, 15, 78, false);
      test_is_umul_overflow_aux(size, 1073742058, 2, true);
      break;
    case 33:
      test_is_umul_overflow_aux(size, 15, 78, false);
      test_is_umul_overflow_aux(size, 4294967530, 4294967530, true);
      break;
    default: assert(false);
  }
}

void
TestBitVector::test_ite(BvFunKind fun_kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    BitVector bv_cond(1, *d_rng);
    BitVector bv_then(size, *d_rng);
    BitVector bv_else(size, *d_rng);
    BitVector res(size);

    if (fun_kind == INPLACE_CHAINABLE)
    {
      // TODO
    }
    else if (fun_kind == INPLACE_NOT_CHAINABLE)
    {
      res.ibvite(bv_cond, bv_then, bv_else);
    }
    else
    {
      res = BitVector::bvite(bv_cond, bv_then, bv_else);
    }

    uint64_t a_cond = bv_cond.to_uint64();
    uint64_t a_then = bv_then.to_uint64();
    uint64_t a_else = bv_else.to_uint64();
    uint64_t a_res  = _ite(a_cond, a_then, a_else, size);
    uint64_t b_res  = res.to_uint64();
    ASSERT_EQ(a_res, b_res);
  }
  BitVector b1(1, *d_rng);
  BitVector b8(8, *d_rng);
  BitVector b16(16, *d_rng);
  if (fun_kind == INPLACE_CHAINABLE)
  {
    // TODO
  }
  else if (fun_kind == INPLACE_NOT_CHAINABLE)
  {
    ASSERT_DEATH(b8.ibvite(b8, b8, b8), "c.d_size == 1");
    ASSERT_DEATH(b8.ibvite(b1, b8, b16), "d_size == e.d_size");
    ASSERT_DEATH(b8.ibvite(b1, b16, b8), "d_size == t.d_size");
  }
  else
  {
    ASSERT_DEATH(BitVector::bvite(b8, b8, b8), "c.d_size == 1");
    ASSERT_DEATH(BitVector::bvite(b1, b8, b16), "t.d_size == e.d_size");
    ASSERT_DEATH(BitVector::bvite(b1, b16, b8), "t.d_size == e.d_size");
  }
}

void
TestBitVector::test_modinv(BvFunKind fun_kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_MODINV_TESTS; ++i)
  {
    BitVector bv(size, *d_rng);
    bv.set_bit(0, 1);  // must be odd
    BitVector res(bv);
    if (fun_kind == INPLACE_CHAINABLE)
    {
      // TODO
    }
    else if (fun_kind == INPLACE_NOT_CHAINABLE)
    {
      res.ibvmodinv(bv);
    }
    else
    {
      res = bv.bvmodinv();
    }
    ASSERT_TRUE(bv.bvmul(res).is_one());
  }
}

void
TestBitVector::test_unary(BvFunKind fun_kind, Kind kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t ares;
    BitVector bv(size, *d_rng);
    BitVector res(bv);
    uint64_t a = bv.to_uint64();
    switch (kind)
    {
      case DEC:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvdec();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvdec(bv);
        }
        else
        {
          res = bv.bvdec();
        }
        ares = _dec(a, size);
        break;

      case INC:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvinc();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvinc(bv);
        }
        else
        {
          res = bv.bvinc();
        }
        ares = _inc(a, size);
        break;

      case NEG:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvneg();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvneg(bv);
        }
        else
        {
          res = bv.bvneg();
        }
        ares = _neg(a, size);
        break;

      case NOT:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvnot();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res.ibvnot(bv);
        }
        else
        {
          res = bv.bvnot();
        }
        ares = _not(a, size);
        break;

      case REDAND:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvredand();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res = BitVector(1);
          res.ibvredand(bv);
        }
        else
        {
          res = bv.bvredand();
        }
        ares = _redand(a, size);
        break;

      case REDOR:
        if (fun_kind == INPLACE_CHAINABLE)
        {
          (void) res.ibvredor();
        }
        else if (fun_kind == INPLACE_NOT_CHAINABLE)
        {
          res = BitVector(1);
          res.ibvredor(bv);
        }
        else
        {
          res = bv.bvredor();
        }
        ares = _redor(a, size);
        break;

      default: assert(false);
    }
    uint64_t bres = res.to_uint64();
    assert(ares == bres);
    ASSERT_EQ(ares, bres);
  }
}

void
TestBitVector::test_binary(BvFunKind fun_kind,
                           TestBitVector::Kind kind,
                           uint32_t size)
{
  BitVector zero = BitVector::mk_zero(size);

  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t ares, bres;
    BitVector bv1(size, *d_rng);
    BitVector bv2(size, *d_rng);
    uint64_t a1 = bv1.to_uint64();
    uint64_t a2 = bv2.to_uint64();

    std::vector<std::pair<BitVector, BitVector>> bv_args = {
        std::make_pair(zero, bv2),
        std::make_pair(bv1, zero),
        std::make_pair(bv1, bv2)};
    std::vector<std::pair<uint64_t, uint64_t>> int_args = {
        std::make_pair(0, a2), std::make_pair(a1, 0), std::make_pair(a1, a2)};

    for (uint32_t i = 0; i < 3; ++i)
    {
      const BitVector& b1 = bv_args[i].first;
      const BitVector& b2 = bv_args[i].second;
      uint64_t i1         = int_args[i].first;
      uint64_t i2         = int_args[i].second;
      BitVector res(b1);
      switch (kind)
      {
        case ADD:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvadd(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvadd(b1, b2);
          }
          else
          {
            res = b1.bvadd(b2);
          }
          ares = _add(i1, i2, size);
          break;

        case AND:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            res.ibvand(b1, b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvand(b1, b2);
          }
          else
          {
            res = b1.bvand(b2);
          }
          ares = _and(i1, i2, size);
          break;

        case ASHR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvashr(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvashr(b1, b2);
          }
          else
          {
            res = b1.bvashr(b2);
          }
          ares = _ashr(i1, i2, size);
          break;

        case EQ:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibveq(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibveq(b1, b2);
          }
          else
          {
            res = b1.bveq(b2);
          }
          ares = _eq(i1, i2, size);
          break;

        case IMPLIES:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvimplies(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvimplies(b1, b2);
          }
          else
          {
            res = b1.bvimplies(b2);
          }
          ares = _implies(i1, i2, size);
          break;

        case MUL:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            // TODO
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvmul(b1, b2);
          }
          else
          {
            res = b1.bvmul(b2);
          }
          ares = _mul(i1, i2, size);
          break;

        case NAND:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            res.ibvnand(b1, b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvnand(b1, b2);
          }
          else
          {
            res = b1.bvnand(b2);
          }
          ares = _nand(i1, i2, size);
          break;

        case NE:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvne(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvne(b1, b2);
          }
          else
          {
            res = b1.bvne(b2);
          }
          ares = _ne(i1, i2, size);
          break;

        case NOR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            res.ibvnor(b1, b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvnor(b1, b2);
          }
          else
          {
            res = b1.bvnor(b2);
          }
          ares = _nor(i1, i2, size);
          break;

        case OR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            res.ibvor(b1, b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvor(b1, b2);
          }
          else
          {
            res = b1.bvor(b2);
          }
          ares = _or(i1, i2, size);
          break;

        case SHL:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvshl(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvshl(b1, b2);
          }
          else
          {
            res = b1.bvshl(b2);
          }
          ares = _shl(i1, i2, size);
          break;

        case SHR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvshr(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvshr(b1, b2);
          }
          else
          {
            res = b1.bvshr(b2);
          }
          ares = _shr(i1, i2, size);
          break;

        case SUB:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            res.ibvsub(b1, b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsub(b1, b2);
          }
          else
          {
            res = b1.bvsub(b2);
          }
          ares = _sub(i1, i2, size);
          break;

        case UDIV:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            // TODO
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvudiv(b1, b2);
          }
          else
          {
            res = b1.bvudiv(b2);
          }
          ares = _udiv(i1, i2, size);
          break;

        case ULT:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvult(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvult(b1, b2);
          }
          else
          {
            res = b1.bvult(b2);
          }
          ares = _ult(i1, i2, size);
          break;

        case ULE:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvule(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvule(b1, b2);
          }
          else
          {
            res = b1.bvule(b2);
          }
          ares = _ule(i1, i2, size);
          break;

        case UGT:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvugt(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvugt(b1, b2);
          }
          else
          {
            res = b1.bvugt(b2);
          }
          ares = _ugt(i1, i2, size);
          break;

        case UGE:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvuge(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvuge(b1, b2);
          }
          else
          {
            res = b1.bvuge(b2);
          }
          ares = _uge(i1, i2, size);
          break;

        case UREM:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            // TODO
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvurem(b1, b2);
          }
          else
          {
            res = b1.bvurem(b2);
          }
          ares = _urem(i1, i2, size);
          break;

        case XOR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvxor(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvxor(b1, b2);
          }
          else
          {
            res = b1.bvxor(b2);
          }
          ares = _xor(i1, i2, size);
          break;

        case XNOR:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvxnor(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvxnor(b1, b2);
          }
          else
          {
            res = b1.bvxnor(b2);
          }
          ares = _xnor(i1, i2, size);
          break;

        default: assert(false);
      }
      bres = res.to_uint64();
      assert(ares == bres);
      ASSERT_EQ(ares, bres);
    }
  }
  BitVector b1(size, *d_rng);
  BitVector b2(size + 1, *d_rng);
  BitVector res(b1);
  /* death tests */
  switch (kind)
  {
    case ADD:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvadd(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvadd(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvadd(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvadd(b2), "d_size == .*d_size");
      }
      break;

    case AND:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvand(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvand(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvand(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvand(b2), "d_size == .*d_size");
      }
      break;

    case ASHR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvashr(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvashr(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvashr(b2), "d_size == .*d_size");
      }
      break;

    case EQ:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibveq(b1, b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibveq(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibveq(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bveq(b2), "d_size == .*d_size");
      }
      break;

    case IMPLIES:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvimplies(b2), "bv1.d_size == 1");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvimplies(b1, b2), "bv1.d_size == 1");
        ASSERT_DEATH(BitVector(1).ibvimplies(b2, b1), "bv0.d_size == 1");
      }
      else
      {
        ASSERT_DEATH(b1.bvimplies(b2), "d_size == .*d_size");
      }
      break;

    case MUL:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvmul(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvmul(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvmul(b2), "d_size == .*d_size");
      }
      break;

    case NAND:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvnand(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvnand(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvnand(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvnand(b2), "d_size == .*d_size");
      }
      break;

    case NE:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvne(b1, b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvne(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvne(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvne(b2), "d_size == .*d_size");
      }
      break;

    case NOR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvnor(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvnor(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvnor(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvnor(b2), "d_size == .*d_size");
      }
      break;

    case OR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvor(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvor(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvor(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvor(b2), "d_size == .*d_size");
      }
      break;

    case SHL:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvshl(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvshl(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvshl(b2), "d_size == .*d_size");
      }
      break;

    case SHR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvshr(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvshr(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvshr(b2), "d_size == .*d_size");
      }
      break;

    case SUB:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvsub(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvsub(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvsub(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsub(b2), "d_size == .*d_size");
      }
      break;

    case UDIV:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvudiv(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvudiv(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvudiv(b2), "d_size == .*d_size");
      }
      break;

    case ULT:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvult(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvult(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvult(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvult(b2), "d_size == .*d_size");
      }
      break;

    case ULE:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvule(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvule(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvule(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvule(b2), "d_size == .*d_size");
      }
      break;

    case UGT:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvugt(b1, b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvugt(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvugt(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvugt(b2), "d_size == .*d_size");
      }
      break;

    case UGE:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvuge(b1, b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvuge(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvuge(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvuge(b2), "d_size == .*d_size");
      }
      break;

    case UREM:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvurem(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvurem(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvurem(b2), "d_size == .*d_size");
      }
      break;

    case XOR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvxor(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvxor(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvxor(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvxor(b2), "d_size == .*d_size");
      }
      break;

    case XNOR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvxnor(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(res.ibvxnor(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(res.ibvxnor(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvxnor(b2), "d_size == .*d_size");
      }
      break;

    default: assert(false);
  }
}

void
TestBitVector::test_binary_signed(BvFunKind fun_kind, Kind kind, uint32_t size)
{
  assert(size < 64);
  BitVector zero = BitVector::mk_zero(size);

  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    int64_t ares, bres;
    BitVector bv1(size, *d_rng);
    BitVector bv2(size, *d_rng);
    int64_t a1 = bv1.to_uint64();
    int64_t a2 = bv2.to_uint64();
    if (bv1.get_bit(size - 1))
    {
      a1 = (UINT64_MAX << size) | a1;
    }
    if (bv2.get_bit(size - 1))
    {
      a2 = (UINT64_MAX << size) | a2;
    }
    std::vector<std::pair<BitVector, BitVector>> bv_args = {
        std::make_pair(zero, bv2),
        std::make_pair(bv1, zero),
        std::make_pair(bv1, bv2)};
    std::vector<std::pair<uint64_t, uint64_t>> int_args = {
        std::make_pair(0, a2), std::make_pair(a1, 0), std::make_pair(a1, a2)};

    for (uint32_t i = 0; i < 3; ++i)
    {
      const BitVector& b1 = bv_args[i].first;
      const BitVector& b2 = bv_args[i].second;
      uint64_t i1         = int_args[i].first;
      uint64_t i2         = int_args[i].second;
      BitVector res(b1);
      switch (kind)
      {
        case SDIV:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            // TODO
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsdiv(b1, b2);
          }
          else
          {
            res = b1.bvsdiv(b2);
          }
          ares = _sdiv(i1, i2, size);
          break;

        case SLT:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvslt(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvslt(b1, b2);
          }
          else
          {
            res = b1.bvslt(b2);
          }
          ares = _slt(i1, i2, size);
          break;

        case SLE:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvsle(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsle(b1, b2);
          }
          else
          {
            res = b1.bvsle(b2);
          }
          ares = _sle(i1, i2, size);
          break;

        case SGT:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvsgt(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsgt(b1, b2);
          }
          else
          {
            res = b1.bvsgt(b2);
          }
          ares = _sgt(i1, i2, size);
          break;

        case SGE:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            (void) res.ibvsge(b2);
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsge(b1, b2);
          }
          else
          {
            res = b1.bvsge(b2);
          }
          ares = _sge(i1, i2, size);
          break;

        case SREM:
          if (fun_kind == INPLACE_CHAINABLE)
          {
            // TODO
          }
          else if (fun_kind == INPLACE_NOT_CHAINABLE)
          {
            res.ibvsrem(b1, b2);
          }
          else
          {
            res = b1.bvsrem(b2);
          }
          ares = _srem(i1, i2, size);
          break;

        default: assert(false);
      }
      bres = res.to_uint64();
      ASSERT_EQ(ares, bres);
    }
  }
  BitVector b1(size, *d_rng);
  BitVector b2(size + 1, *d_rng);
  /* death tests */
  switch (kind)
  {
    case SDIV:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(size).ibvsdiv(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(size).ibvsdiv(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsdiv(b2), "d_size == .*d_size");
      }
      break;

    case SLT:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvslt(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvslt(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvslt(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvslt(b2), "d_size == .*d_size");
      }
      break;

    case SLE:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvsle(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvsle(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvsle(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsle(b2), "d_size == .*d_size");
      }
      break;

    case SGT:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvsgt(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvsgt(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvsgt(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsgt(b2), "d_size == .*d_size");
      }
      break;

    case SGE:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        ASSERT_DEATH(b1.ibvsge(b2), "d_size == .*d_size");
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(1).ibvsge(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(1).ibvsge(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsge(b2), "d_size == .*d_size");
      }
      break;

    case SREM:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        // TODO
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        ASSERT_DEATH(BitVector(size).ibvsrem(b1, b2), "d_size == .*d_size");
        ASSERT_DEATH(BitVector(size).ibvsrem(b2, b1), "d_size == .*d_size");
      }
      else
      {
        ASSERT_DEATH(b1.bvsrem(b2), "d_size == .*d_size");
      }
      break;

    default: assert(false);
  }
}

void
TestBitVector::test_concat(BvFunKind fun_kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    uint32_t size1 = d_rng->pick<uint32_t>(1, size - 1);
    uint32_t size2 = size - size1;
    BitVector bv1(size1, *d_rng);
    BitVector bv2(size2, *d_rng);
    BitVector res(size);
    if (fun_kind == INPLACE_CHAINABLE)
    {
      // TODO
    }
    else if (fun_kind == INPLACE_NOT_CHAINABLE)
    {
      res.ibvconcat(bv1, bv2);
    }
    else
    {
      res = bv1.bvconcat(bv2);
    }
    ASSERT_EQ(res.size(), size1 + size2);
    uint64_t u1   = bv1.to_uint64();
    uint64_t u2   = bv2.to_uint64();
    uint64_t u    = (u1 << size2) | u2;
    uint64_t ures = res.to_uint64();
    ASSERT_EQ(u, ures);
  }
}

void
TestBitVector::test_extract(BvFunKind fun_kind, uint32_t size)
{
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    BitVector bv(size, *d_rng);
    uint32_t lo = rand() % size;
    uint32_t hi = rand() % (size - lo) + lo;
    ASSERT_GE(hi, lo);
    ASSERT_LT(hi, size);
    ASSERT_LT(lo, size);

    BitVector res(hi - lo + 1);
    if (fun_kind == INPLACE_CHAINABLE)
    {
      // TODO
    }
    else if (fun_kind == INPLACE_NOT_CHAINABLE)
    {
      res.ibvextract(bv, hi, lo);
    }
    else
    {
      res = bv.bvextract(hi, lo);
    }
    ASSERT_EQ(res.size(), hi - lo + 1);
    std::string res_str = res.to_string();
    std::string bv_str  = bv.to_string();
    uint32_t len        = hi - lo + 1;
    ASSERT_EQ(bv_str.compare(size - hi - 1, len, res_str, 0, len), 0);
  }
  if (size > 1)
  {
    ASSERT_DEATH(BitVector(size, *d_rng).bvextract(size - 2, size - 1),
                 "idx_hi >= idx_lo");
  }
}

void
TestBitVector::test_shift_aux(BvFunKind fun_kind,
                              Kind kind,
                              const std::string& to_shift,
                              const std::string& shift,
                              const std::string& expected,
                              bool shift_by_int)
{
  uint32_t size = to_shift.size();
  assert(size == shift.size());
  assert(size == expected.size());

  BitVector bv(to_shift.size(), to_shift);
  BitVector bv_shift(shift.size(), shift);
  BitVector bv_expected(expected.size(), expected);
  BitVector res(bv);
  uint32_t int_shift = strtoul(shift.c_str(), nullptr, 2);
  switch (kind)
  {
    case ASHR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        (void) res.ibvashr(bv_shift);
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        res.ibvashr(bv, bv_shift);
      }
      else
      {
        res = bv.bvashr(bv_shift);
      }
      break;
    case SHL:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        (void) res.ibvshl(bv_shift);
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        if (shift_by_int)
        {
          res.ibvshl(bv, int_shift);
        }
        else
        {
          res.ibvshl(bv, bv_shift);
        }
      }
      else
      {
        if (shift_by_int)
        {
          res = bv.bvshl(int_shift);
        }
        else
        {
          res = bv.bvshl(bv_shift);
        }
      }
      break;
    case SHR:
      if (fun_kind == INPLACE_CHAINABLE)
      {
        (void) res.ibvshr(bv_shift);
      }
      else if (fun_kind == INPLACE_NOT_CHAINABLE)
      {
        if (shift_by_int)
        {
          res.ibvshr(bv, int_shift);
        }
        else
        {
          res.ibvshr(bv, bv_shift);
        }
      }
      else
      {
        if (shift_by_int)
        {
          res = bv.bvshr(int_shift);
        }
        else
        {
          res = bv.bvshr(bv_shift);
        }
      }
      break;
    default: assert(false);
  }

  assert(res.compare(bv_expected) == 0);
  ASSERT_EQ(res.compare(bv_expected), 0);
}

void
TestBitVector::test_shift(BvFunKind fun_kind, Kind kind, bool shift_by_int)
{
  for (uint32_t i = 0, size = 2; i < (1u << size); ++i)
  {
    for (uint32_t j = 0; j < (1u << size); ++j)
    {
      std::stringstream ss_expected;
      if (kind == SHL)
      {
        ss_expected << std::bitset<2>(i).to_string() << std::string(j, '0');
      }
      else if (kind == SHR)
      {
        ss_expected << std::string(j, '0') << std::bitset<2>(i).to_string();
      }
      else
      {
        assert(kind == ASHR);
        std::bitset<2> bits_i(i);
        ss_expected << std::string(j, bits_i[size - 1] == 1 ? '1' : '0')
                    << bits_i.to_string();
      }
      std::string expected = ss_expected.str();
      if (kind == SHL)
      {
        expected = expected.substr(expected.size() - size, size);
      }
      else
      {
        expected = expected.substr(0, size);
      }
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<2>(i).to_string().c_str(),
                     std::bitset<2>(j).to_string().c_str(),
                     expected.c_str(),
                     shift_by_int);
    }
  }

  for (uint32_t i = 0, size = 3; i < (1u << size); ++i)
  {
    for (uint32_t j = 0; j < (1u << size); ++j)
    {
      std::stringstream ss_expected;
      if (kind == SHL)
      {
        ss_expected << std::bitset<3>(i).to_string() << std::string(j, '0');
      }
      else if (kind == SHR)
      {
        ss_expected << std::string(j, '0') << std::bitset<3>(i).to_string();
      }
      else
      {
        assert(kind == ASHR);
        std::bitset<3> bits_i(i);
        ss_expected << std::string(j, bits_i[size - 1] == 1 ? '1' : '0')
                    << bits_i.to_string();
      }
      std::string expected = ss_expected.str();
      if (kind == SHL)
      {
        expected = expected.substr(expected.size() - size, size);
      }
      else
      {
        expected = expected.substr(0, size);
      }
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<3>(i).to_string().c_str(),
                     std::bitset<3>(j).to_string().c_str(),
                     expected.c_str(),
                     shift_by_int);
    }
  }

  for (uint32_t i = 0, size = 8; i < (1u << size); ++i)
  {
    for (uint32_t j = 0; j < (1u << size); ++j)
    {
      std::stringstream ss_expected;
      if (kind == SHL)
      {
        ss_expected << std::bitset<8>(i).to_string() << std::string(j, '0');
      }
      else if (kind == SHR)
      {
        ss_expected << std::string(j, '0') << std::bitset<8>(i).to_string();
      }
      else
      {
        assert(kind == ASHR);
        std::bitset<8> bits_i(i);
        ss_expected << std::string(j, bits_i[size - 1] == 1 ? '1' : '0')
                    << bits_i.to_string();
      }
      std::string expected = ss_expected.str();
      if (kind == SHL)
      {
        expected = expected.substr(expected.size() - size, size);
      }
      else
      {
        expected = expected.substr(0, size);
      }
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<8>(i).to_string().c_str(),
                     std::bitset<8>(j).to_string().c_str(),
                     expected.c_str(),
                     shift_by_int);
    }
  }

  for (uint32_t i = 0, size = 65; i < (1u << size); ++i)
  {
    /* shift value fits into uint64_t */
    for (uint64_t j = 0; j < 32; ++j)
    {
      std::stringstream ss_expected;
      if (kind == SHL)
      {
        ss_expected << std::bitset<65>(i).to_string() << std::string(j, '0');
      }
      else if (kind == SHR)
      {
        ss_expected << std::string(j, '0') << std::bitset<65>(i).to_string();
      }
      else
      {
        assert(kind == ASHR);
        std::bitset<65> bits_i(i);
        ss_expected << std::string(j, bits_i[size - 1] == 1 ? '1' : '0')
                    << bits_i.to_string();
      }
      std::string expected = ss_expected.str();
      if (kind == SHL)
      {
        expected = expected.substr(expected.size() - size, size);
      }
      else
      {
        expected = expected.substr(0, size);
      }
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<65>(i).to_string().c_str(),
                     std::bitset<65>(j).to_string().c_str(),
                     expected.c_str(),
                     shift_by_int);
    }
    /* shift value doesn't fit into uint64_t */
    {
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<65>(i).to_string().c_str(),
                     std::bitset<65>(0u).set(64, 1).to_string().c_str(),
                     std::string(size, '0').c_str(),
                     shift_by_int);
    }
  }

  for (uint32_t i = 0, size = 128; i < (1u << size); ++i)
  {
    /* shift value fits into uint64_t */
    for (uint64_t j = 0; j < 32; ++j)
    {
      std::stringstream ss_expected;
      if (kind == SHL)
      {
        ss_expected << std::bitset<128>(i).to_string() << std::string(j, '0');
      }
      else if (kind == SHR)
      {
        ss_expected << std::string(j, '0') << std::bitset<128>(i).to_string();
      }
      else
      {
        assert(kind == ASHR);
        std::bitset<128> bits_i(i);
        ss_expected << std::string(j, bits_i[size - 1] == 1 ? '1' : '0')
                    << bits_i.to_string();
      }
      std::string expected = ss_expected.str();
      if (kind == SHL)
      {
        expected = expected.substr(expected.size() - size, size);
      }
      else
      {
        expected = expected.substr(0, size);
      }
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<128>(i).to_string().c_str(),
                     std::bitset<128>(j).to_string().c_str(),
                     expected.c_str(),
                     shift_by_int);
    }
    /* shift value doesn't fit into uint64_t */
    for (uint64_t j = 64; j < 128; ++j)
    {
      test_shift_aux(fun_kind,
                     kind,
                     std::bitset<128>(i).to_string().c_str(),
                     std::bitset<128>(0u).set(j, 1).to_string().c_str(),
                     std::string(size, '0').c_str(),
                     shift_by_int);
    }
  }
}

void
TestBitVector::test_udivurem(uint32_t size)
{
  BitVector zero = BitVector::mk_zero(size);
  for (uint32_t i = 0; i < N_TESTS; ++i)
  {
    BitVector q, r;
    BitVector bv1(size, *d_rng);
    BitVector bv2(size, *d_rng);
    uint64_t a1 = bv1.to_uint64();
    uint64_t a2 = bv2.to_uint64();
    uint64_t ares_div, ares_rem, bres_div, bres_rem;
    /* test for x = 0 explicitly */
    zero.bvudivurem(bv2, &q, &r);
    ares_div = _udiv(0, a2, size);
    ares_rem = _urem(0, a2, size);
    bres_div = q.to_uint64();
    bres_rem = r.to_uint64();
    ASSERT_EQ(ares_div, bres_div);
    ASSERT_EQ(ares_rem, bres_rem);
    /* test for y = 0 explicitly */
    bv1.bvudivurem(zero, &q, &r);
    ares_div = _udiv(a1, 0, size);
    ares_rem = _urem(a1, 0, size);
    bres_div = q.to_uint64();
    bres_rem = r.to_uint64();
    ASSERT_EQ(ares_div, bres_div);
    ASSERT_EQ(ares_rem, bres_rem);
    /* test x, y random */
    bv1.bvudivurem(bv2, &q, &r);
    ares_div = _udiv(a1, a2, size);
    ares_rem = _urem(a1, a2, size);
    bres_div = q.to_uint64();
    bres_rem = r.to_uint64();
    ASSERT_EQ(ares_div, bres_div);
    ASSERT_EQ(ares_rem, bres_rem);
  }
}

/* -------------------------------------------------------------------------- */

TEST_F(TestBitVector, ctor_dtor)
{
  ASSERT_NO_FATAL_FAILURE(BitVector(1));
  ASSERT_NO_FATAL_FAILURE(BitVector(10));
  ASSERT_NO_FATAL_FAILURE(BitVector(6, "101010"));
  ASSERT_NO_FATAL_FAILURE(BitVector(8, "101010"));
  ASSERT_NO_FATAL_FAILURE(BitVector(16, 1234));
  ASSERT_NO_FATAL_FAILURE(BitVector(16, 123412341234));
  ASSERT_DEATH(BitVector(0), "> 0");
  ASSERT_DEATH(BitVector(2, "101010"), "<= size");
  ASSERT_DEATH(BitVector(2, ""), "empty");
  ASSERT_DEATH(BitVector(6, "123412"), "is_bin_str");
  ASSERT_DEATH(BitVector(0, 1234), "> 0");
}

TEST_F(TestBitVector, ctor_rand)
{
  for (uint32_t size = 1; size <= 64; ++size)
  {
    BitVector bv1(size, *d_rng);
    BitVector bv2(size, *d_rng);
    BitVector bv3(size, *d_rng);
    assert(bv1.compare(bv2) || bv1.compare(bv3) || bv2.compare(bv3));
  }
}

TEST_F(TestBitVector, ctor_random_range)
{
  for (uint32_t size = 1; size <= 64; ++size)
  {
    BitVector from(size, *d_rng);
    // from == to
    BitVector bv1(size, *d_rng, from, from);
    ASSERT_EQ(bv1.to_uint64(), from.to_uint64());
    // from < to
    BitVector to(size, *d_rng);
    while (from.compare(to) == 0)
    {
      to = BitVector(size, *d_rng);
    }
    if (to.to_uint64() < from.to_uint64())
    {
      BitVector tmp = to;
      to            = from;
      from          = tmp;
    }

    BitVector bv2(size, *d_rng, from, to);
    ASSERT_GE(bv2.to_uint64(), from.to_uint64());
    ASSERT_LE(bv2.to_uint64(), to.to_uint64());
  }
}

TEST_F(TestBitVector, ctor_random_signed_range)
{
  for (uint32_t size = 1; size <= 64; size++)
  {
    BitVector from(size, *d_rng);
    // from == to
    BitVector bv1(size, *d_rng, from, from, true);
    assert(bv1.to_uint64() == from.to_uint64());
    ASSERT_EQ(bv1.to_uint64(), from.to_uint64());
    // from < to
    BitVector to(size, *d_rng);
    while (from.signed_compare(to) == 0)
    {
      to = BitVector(size, *d_rng);
    }
    if (from.signed_compare(to) >= 0)
    {
      BitVector tmp = to;
      to            = from;
      from          = tmp;
    }
    BitVector bv2(size, *d_rng, from, to, true);
    ASSERT_LE(from.signed_compare(bv2), 0);
    ASSERT_LE(bv2.signed_compare(to), 0);
  }
}

TEST_F(TestBitVector, ctor_random_bit_range)
{
  test_ctor_random_bit_range(1);
  test_ctor_random_bit_range(7);
  test_ctor_random_bit_range(31);
  test_ctor_random_bit_range(33);
}

TEST_F(TestBitVector, to_string)
{
  ASSERT_EQ(BitVector(1).to_string(), "0");
  ASSERT_EQ(BitVector(10).to_string(), "0000000000");
  ASSERT_EQ(BitVector(6, "101010").to_string(), "101010");
  ASSERT_EQ(BitVector(8, "101010").to_string(), "00101010");
  ASSERT_EQ(BitVector(16, 1234).to_string(), "0000010011010010");
  ASSERT_EQ(BitVector(16, 123412341234).to_string(), "1110000111110010");
  ASSERT_EQ(BitVector(16, UINT64_MAX).to_string(), "1111111111111111");
}

TEST_F(TestBitVector, to_uint64)
{
  for (uint64_t i = 0; i < N_TESTS; ++i)
  {
    uint64_t x = (d_rng->pick<uint64_t>() << 32) | d_rng->pick<uint64_t>();
    BitVector bv(64, x);
    uint64_t y = bv.to_uint64();
    ASSERT_EQ(x, y);
  }
  ASSERT_NO_FATAL_FAILURE(BitVector(28).to_uint64());
  ASSERT_DEATH(BitVector(128).to_uint64(), "d_size <= 64");
}

TEST_F(TestBitVector, compare)
{
  for (uint32_t i = 0; i < 15; ++i)
  {
    BitVector bv1(4, i);
    BitVector bv2(4, i);
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_TRUE(bv1 == bv2);
  }

  for (uint32_t i = 0; i < 15 - 1; ++i)
  {
    BitVector bv1(4, i);
    BitVector bv2(4, i + 1);
    ASSERT_LT(bv1.compare(bv2), 0);
    ASSERT_GT(bv2.compare(bv1), 0);
    ASSERT_FALSE(bv1 == bv2);
    ASSERT_TRUE(bv1 != bv2);
  }

  for (uint32_t i = 0, j = 0; i < 15; ++i)
  {
    uint32_t k = rand() % 16;
    do
    {
      j = rand() % 16;
    } while (j == k);

    BitVector bv1(4, j);
    BitVector bv2(4, k);
    if (j > k)
    {
      ASSERT_GT(bv1.compare(bv2), 0);
      ASSERT_LT(bv2.compare(bv1), 0);
      ASSERT_FALSE(bv1 == bv2);
      ASSERT_TRUE(bv1 != bv2);
    }
    if (j < k)
    {
      ASSERT_LT(bv1.compare(bv2), 0);
      ASSERT_GT(bv2.compare(bv1), 0);
      ASSERT_FALSE(bv1 == bv2);
      ASSERT_TRUE(bv1 != bv2);
    }
  }
  ASSERT_DEATH(BitVector(1).compare(BitVector(2)), "");
}

TEST_F(TestBitVector, signed_compare)
{
  for (int32_t i = -8; i < 7; ++i)
  {
    BitVector bv1(4, i);
    BitVector bv2(4, i);
    ASSERT_EQ(bv1.signed_compare(bv2), 0);
    ASSERT_TRUE(bv1 == bv2);
  }

  for (int32_t i = -8; i < 7 - 1; i++)
  {
    BitVector bv1(4, i);
    BitVector bv2(4, i + 1);
    ASSERT_LT(bv1.signed_compare(bv2), 0);
    ASSERT_GT(bv2.signed_compare(bv1), 0);
    ASSERT_FALSE(bv1 == bv2);
    ASSERT_TRUE(bv1 != bv2);
  }

  for (int32_t i = 0, j = 0; i < 15; i++)
  {
    /* j <= 0, k <= 0 */
    int32_t k = rand() % 9;
    do
    {
      j = rand() % 9;
    } while (j == k);
    j = -j;
    k = -k;
    BitVector bv1(4, j);
    BitVector bv2(4, k);
    if (j > k)
    {
      ASSERT_GT(bv1.signed_compare(bv2), 0);
      ASSERT_LT(bv2.signed_compare(bv1), 0);
      ASSERT_FALSE(bv1 == bv2);
      ASSERT_TRUE(bv1 != bv2);
    }
    if (j < k)
    {
      ASSERT_LT(bv1.signed_compare(bv2), 0);
      ASSERT_GT(bv2.signed_compare(bv1), 0);
      ASSERT_FALSE(bv1 == bv2);
      ASSERT_TRUE(bv1 != bv2);
    }

    {
      /* j <= 0, k >= 0 */
      k = rand() % 8;
      do
      {
        j = rand() % 9;
      } while (j == k);
      j = -j;
      BitVector bv1(4, j);
      BitVector bv2(4, k);
      if (j > k)
      {
        ASSERT_GT(bv1.signed_compare(bv2), 0);
        ASSERT_LT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
      if (j < k)
      {
        ASSERT_LT(bv1.signed_compare(bv2), 0);
        ASSERT_GT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
    }

    {
      /* j >= 0, k <= 0 */
      k = rand() % 9;
      do
      {
        j = rand() % 8;
      } while (j == k);
      k = -k;
      BitVector bv1(4, j);
      BitVector bv2(4, k);
      if (j > k)
      {
        ASSERT_GT(bv1.signed_compare(bv2), 0);
        ASSERT_LT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
      if (j < k)
      {
        ASSERT_LT(bv1.signed_compare(bv2), 0);
        ASSERT_GT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
    }

    {
      /* j >= 0, k >= 0 */
      k = rand() % 8;
      do
      {
        j = rand() % 8;
      } while (j == k);
      BitVector bv1(4, -j);
      BitVector bv2(4, -k);
      if (-j > -k)
      {
        ASSERT_GT(bv1.signed_compare(bv2), 0);
        ASSERT_LT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
      if (-j < -k)
      {
        ASSERT_LT(bv1.signed_compare(bv2), 0);
        ASSERT_GT(bv2.signed_compare(bv1), 0);
        ASSERT_FALSE(bv1 == bv2);
        ASSERT_TRUE(bv1 != bv2);
      }
    }
  }
  ASSERT_DEATH(BitVector(1).signed_compare(BitVector(2)),
               "d_size == bv.d_size");
}

TEST_F(TestBitVector, is_true)
{
  BitVector bv1 = BitVector::mk_true();
  ASSERT_TRUE(bv1.is_true());
  for (int32_t i = 1; i < 32; ++i)
  {
    BitVector bv2 = BitVector::mk_one(i);
    BitVector bv3(i, d_rng->pick<uint32_t>(1, (1 << i) - 1));
    if (i > 1)
    {
      ASSERT_FALSE(bv2.is_true());
      ASSERT_FALSE(bv3.is_true());
    }
    else
    {
      ASSERT_TRUE(bv3.is_true());
      ASSERT_TRUE(bv3.is_true());
    }
  }
}

TEST_F(TestBitVector, is_false)
{
  BitVector bv1 = BitVector::mk_false();
  ASSERT_TRUE(bv1.is_false());
  for (int32_t i = 1; i < 32; ++i)
  {
    BitVector bv2 = BitVector::mk_zero(i);
    BitVector bv3(i, d_rng->pick<uint32_t>(1, (1 << i) - 1));
    if (i > 1)
    {
      ASSERT_FALSE(bv2.is_false());
      ASSERT_FALSE(bv3.is_false());
    }
    else
    {
      ASSERT_TRUE(bv2.is_false());
      ASSERT_FALSE(bv3.is_false());
    }
  }
}

TEST_F(TestBitVector, set_get_flip_bit)
{
  for (uint32_t i = 1; i < 32; ++i)
  {
    BitVector bv(i, *d_rng);
    uint32_t n  = d_rng->pick<uint32_t>(0, i - 1);
    uint32_t v  = bv.get_bit(n);
    uint32_t vv = d_rng->flip_coin() ? 1 : 0;
    bv.set_bit(n, vv);
    ASSERT_EQ(bv.get_bit(n), vv);
    ASSERT_TRUE(v == vv || bv.get_bit(n) == (((~v) << 31) >> 31));
    bv.flip_bit(n);
    ASSERT_EQ(bv.get_bit(n), (((~vv) << 31) >> 31));
  }
  ASSERT_DEATH(BitVector(5).get_bit(5), "< size");
}

TEST_F(TestBitVector, is_zero)
{
  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '0');
    BitVector bv1 = BitVector::mk_zero(i);
    BitVector bv2(i, s);
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 0);
    }
    else
    {
      BitVector r(64, 0);
      BitVector l(i - 64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_TRUE(bv1.is_zero());
    ASSERT_TRUE(bv2.is_zero());
    ASSERT_TRUE(bv3.is_zero());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << std::string(i - 1, '0') << "1";
    BitVector bv1 = BitVector::mk_one(i);
    BitVector bv2(i, ss.str());
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 1);
    }
    else
    {
      BitVector r(i - 64, 1);
      BitVector l(64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_zero());
    ASSERT_FALSE(bv2.is_zero());
    ASSERT_FALSE(bv3.is_zero());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '1');
    BitVector bv1 = BitVector::mk_ones(i);
    BitVector bv2(i, s);
    BitVector bv3 = mk_ones(i);
    ASSERT_FALSE(bv1.is_zero());
    ASSERT_FALSE(bv2.is_zero());
    ASSERT_FALSE(bv3.is_zero());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "1" << std::string(i - 1, '0');
    BitVector bv1 = BitVector::mk_min_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_min_signed(i);
    ASSERT_FALSE(bv1.is_zero());
    ASSERT_FALSE(bv2.is_zero());
    ASSERT_FALSE(bv3.is_zero());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "0" << std::string(i - 1, '1');
    BitVector bv1 = BitVector::mk_max_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_max_signed(i);
    ASSERT_FALSE(bv1.is_zero());
    ASSERT_FALSE(bv2.is_zero());
    ASSERT_FALSE(bv3.is_zero());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }
}

TEST_F(TestBitVector, is_one)
{
  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '0');
    BitVector bv1 = BitVector::mk_zero(i);
    BitVector bv2(i, s);
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 0);
    }
    else
    {
      BitVector r(64, 0);
      BitVector l(i - 64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_one());
    ASSERT_FALSE(bv2.is_one());
    ASSERT_FALSE(bv3.is_one());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << std::string(i - 1, '0') << "1";
    BitVector bv1 = BitVector::mk_one(i);
    BitVector bv2(i, ss.str());
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 1);
    }
    else
    {
      BitVector r(i - 64, 1);
      BitVector l(64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_TRUE(bv1.is_one());
    ASSERT_TRUE(bv2.is_one());
    ASSERT_TRUE(bv3.is_one());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::string s(i, '1');
    BitVector bv1 = BitVector::mk_ones(i);
    BitVector bv2(i, s);
    BitVector bv3 = mk_ones(i);
    ASSERT_FALSE(bv1.is_one());
    ASSERT_FALSE(bv2.is_one());
    ASSERT_FALSE(bv3.is_one());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "1" << std::string(i - 1, '0');
    BitVector bv1 = BitVector::mk_min_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_min_signed(i);
    ASSERT_FALSE(bv1.is_one());
    ASSERT_FALSE(bv2.is_one());
    ASSERT_FALSE(bv3.is_one());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 3; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "0" << std::string(i - 1, '1');
    BitVector bv1 = BitVector::mk_max_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_max_signed(i);
    ASSERT_FALSE(bv1.is_one());
    ASSERT_FALSE(bv2.is_one());
    ASSERT_FALSE(bv3.is_one());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }
}

TEST_F(TestBitVector, is_ones)
{
  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '0');
    BitVector bv1 = BitVector::mk_zero(i);
    BitVector bv2(i, s);
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 0);
    }
    else
    {
      BitVector r(64, 0);
      BitVector l(i - 64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_ones());
    ASSERT_FALSE(bv2.is_ones());
    ASSERT_FALSE(bv3.is_ones());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << std::string(i - 1, '0') << "1";
    BitVector bv1 = BitVector::mk_one(i);
    BitVector bv2(i, ss.str());
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 1);
    }
    else
    {
      BitVector r(i - 64, 1);
      BitVector l(64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_ones());
    ASSERT_FALSE(bv2.is_ones());
    ASSERT_FALSE(bv3.is_ones());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '1');
    BitVector bv1 = BitVector::mk_ones(i);
    BitVector bv2(i, s);
    BitVector bv3 = mk_ones(i);
    ASSERT_TRUE(bv1.is_ones());
    ASSERT_TRUE(bv2.is_ones());
    ASSERT_TRUE(bv3.is_ones());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "1" << std::string(i - 1, '0');
    BitVector bv1 = BitVector::mk_min_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_min_signed(i);
    ASSERT_FALSE(bv1.is_ones());
    ASSERT_FALSE(bv2.is_ones());
    ASSERT_FALSE(bv3.is_ones());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "0" << std::string(i - 1, '1');
    BitVector bv1 = BitVector::mk_max_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_max_signed(i);
    ASSERT_FALSE(bv1.is_ones());
    ASSERT_FALSE(bv2.is_ones());
    ASSERT_FALSE(bv3.is_ones());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }
}

TEST_F(TestBitVector, is_max_signed)
{
  for (uint32_t i = 2; i <= 128; i++)
  {
    std::string s(i, '0');
    BitVector bv1 = BitVector::mk_zero(i);
    BitVector bv2(i, s);
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 0);
    }
    else
    {
      BitVector r(64, 0);
      BitVector l(i - 64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_max_signed());
    ASSERT_FALSE(bv2.is_max_signed());
    ASSERT_FALSE(bv3.is_max_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 3; i <= 128; i++)
  {
    std::stringstream ss;
    ss << std::string(i - 1, '0') << "1";
    BitVector bv1 = BitVector::mk_one(i);
    BitVector bv2(i, ss.str());
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 1);
    }
    else
    {
      BitVector r(i - 64, 1);
      BitVector l(64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_max_signed());
    ASSERT_FALSE(bv2.is_max_signed());
    ASSERT_FALSE(bv3.is_max_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '1');
    BitVector bv1 = BitVector::mk_ones(i);
    BitVector bv2(i, s);
    BitVector bv3 = mk_ones(i);
    ASSERT_FALSE(bv1.is_max_signed());
    ASSERT_FALSE(bv2.is_max_signed());
    ASSERT_FALSE(bv3.is_max_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "1" << std::string(i - 1, '0');
    BitVector bv1 = BitVector::mk_min_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_min_signed(i);
    ASSERT_FALSE(bv1.is_max_signed());
    ASSERT_FALSE(bv2.is_max_signed());
    ASSERT_FALSE(bv3.is_max_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "0" << std::string(i - 1, '1');
    BitVector bv1 = BitVector::mk_max_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_max_signed(i);
    ASSERT_TRUE(bv1.is_max_signed());
    ASSERT_TRUE(bv2.is_max_signed());
    ASSERT_TRUE(bv3.is_max_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }
}

TEST_F(TestBitVector, is_min_signed)
{
  for (uint32_t i = 1; i <= 128; i++)
  {
    std::string s(i, '0');
    BitVector bv1 = BitVector::mk_zero(i);
    BitVector bv2(i, s);
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 0);
    }
    else
    {
      BitVector r(64, 0);
      BitVector l(i - 64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_min_signed());
    ASSERT_FALSE(bv2.is_min_signed());
    ASSERT_FALSE(bv3.is_min_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::stringstream ss;
    ss << std::string(i - 1, '0') << "1";
    BitVector bv1 = BitVector::mk_one(i);
    BitVector bv2(i, ss.str());
    BitVector bv3;
    if (i <= 64)
    {
      bv3 = BitVector(i, 1);
    }
    else
    {
      BitVector r(i - 64, 1);
      BitVector l(64, 0);
      bv3 = l.bvconcat(r);
    }
    ASSERT_FALSE(bv1.is_min_signed());
    ASSERT_FALSE(bv2.is_min_signed());
    ASSERT_FALSE(bv3.is_min_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 2; i <= 128; i++)
  {
    std::string s(i, '1');
    BitVector bv1 = BitVector::mk_ones(i);
    BitVector bv2(i, s);
    BitVector bv3 = mk_ones(i);
    ASSERT_FALSE(bv1.is_min_signed());
    ASSERT_FALSE(bv2.is_min_signed());
    ASSERT_FALSE(bv3.is_min_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "1" << std::string(i - 1, '0');
    BitVector bv1 = BitVector::mk_min_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_min_signed(i);
    ASSERT_TRUE(bv1.is_min_signed());
    ASSERT_TRUE(bv2.is_min_signed());
    ASSERT_TRUE(bv3.is_min_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }

  for (uint32_t i = 1; i <= 128; i++)
  {
    std::stringstream ss;
    ss << "0" << std::string(i - 1, '1');
    BitVector bv1 = BitVector::mk_max_signed(i);
    BitVector bv2(i, ss.str());
    BitVector bv3 = mk_max_signed(i);
    ASSERT_FALSE(bv1.is_min_signed());
    ASSERT_FALSE(bv2.is_min_signed());
    ASSERT_FALSE(bv3.is_min_signed());
    ASSERT_EQ(bv1.compare(bv2), 0);
    ASSERT_EQ(bv1.compare(bv3), 0);
  }
}

TEST_F(TestBitVector, count_trailing_zeros)
{
  test_count(8, false, true);
  test_count(64, false, true);
  test_count(76, false, true);
  test_count(128, false, true);
  test_count(176, false, true);
}

TEST_F(TestBitVector, count_leading_zeros)
{
  test_count(8, true, true);
  test_count(64, true, true);
  test_count(76, true, true);
  test_count(128, true, true);
  test_count(176, true, true);
}

TEST_F(TestBitVector, count_leading_ones)
{
  test_count(8, true, false);
  test_count(64, true, false);
  test_count(76, true, false);
  test_count(128, true, false);
  test_count(176, true, false);
}

/* -------------------------------------------------------------------------- */

TEST_F(TestBitVector, dec)
{
  test_unary(DEFAULT, DEC, 1);
  test_unary(DEFAULT, DEC, 7);
  test_unary(DEFAULT, DEC, 31);
  test_unary(DEFAULT, DEC, 33);
}

TEST_F(TestBitVector, inc)
{
  test_unary(DEFAULT, INC, 1);
  test_unary(DEFAULT, INC, 7);
  test_unary(DEFAULT, INC, 31);
  test_unary(DEFAULT, INC, 33);
}

TEST_F(TestBitVector, neg)
{
  test_unary(DEFAULT, NEG, 1);
  test_unary(DEFAULT, NEG, 7);
  test_unary(DEFAULT, NEG, 31);
  test_unary(DEFAULT, NEG, 33);
}

TEST_F(TestBitVector, not )
{
  test_unary(DEFAULT, NOT, 1);
  test_unary(DEFAULT, NOT, 7);
  test_unary(DEFAULT, NOT, 31);
  test_unary(DEFAULT, NOT, 33);
}

TEST_F(TestBitVector, redand)
{
  test_unary(DEFAULT, REDAND, 1);
  test_unary(DEFAULT, REDAND, 7);
  test_unary(DEFAULT, REDAND, 31);
  test_unary(DEFAULT, REDAND, 33);
}

TEST_F(TestBitVector, redor)
{
  test_unary(DEFAULT, REDOR, 1);
  test_unary(DEFAULT, REDOR, 7);
  test_unary(DEFAULT, REDOR, 31);
  test_unary(DEFAULT, REDOR, 33);
}

TEST_F(TestBitVector, add)
{
  test_binary(DEFAULT, ADD, 1);
  test_binary(DEFAULT, ADD, 7);
  test_binary(DEFAULT, ADD, 31);
  test_binary(DEFAULT, ADD, 33);
}

TEST_F(TestBitVector, and)
{
  test_binary(DEFAULT, AND, 1);
  test_binary(DEFAULT, AND, 7);
  test_binary(DEFAULT, AND, 31);
  test_binary(DEFAULT, AND, 33);
}

TEST_F(TestBitVector, concat)
{
  test_concat(DEFAULT, 2);
  test_concat(DEFAULT, 7);
  test_concat(DEFAULT, 31);
  test_concat(DEFAULT, 33);
  test_concat(DEFAULT, 64);
}

TEST_F(TestBitVector, eq)
{
  test_binary(DEFAULT, EQ, 1);
  test_binary(DEFAULT, EQ, 7);
  test_binary(DEFAULT, EQ, 31);
  test_binary(DEFAULT, EQ, 33);
}

TEST_F(TestBitVector, extract)
{
  test_extract(DEFAULT, 1);
  test_extract(DEFAULT, 7);
  test_extract(DEFAULT, 31);
  test_extract(DEFAULT, 33);
}

TEST_F(TestBitVector, implies) { test_binary(DEFAULT, IMPLIES, 1); }

TEST_F(TestBitVector, is_uadd_overflow)
{
  test_is_uadd_overflow(1);
  test_is_uadd_overflow(7);
  test_is_uadd_overflow(31);
  test_is_uadd_overflow(33);
}

TEST_F(TestBitVector, is_umul_overflow)
{
  test_is_umul_overflow(1);
  test_is_umul_overflow(7);
  test_is_umul_overflow(31);
  test_is_umul_overflow(33);
}

TEST_F(TestBitVector, ite)
{
  test_ite(DEFAULT, 1);
  test_ite(DEFAULT, 7);
  test_ite(DEFAULT, 31);
  test_ite(DEFAULT, 33);
}

TEST_F(TestBitVector, modinv)
{
  test_ite(DEFAULT, 1);
  test_ite(DEFAULT, 7);
  test_ite(DEFAULT, 31);
  test_ite(DEFAULT, 33);
}

TEST_F(TestBitVector, mul)
{
  test_binary(DEFAULT, MUL, 1);
  test_binary(DEFAULT, MUL, 7);
  test_binary(DEFAULT, MUL, 31);
  test_binary(DEFAULT, MUL, 33);
}

TEST_F(TestBitVector, nand)
{
  test_binary(DEFAULT, NAND, 1);
  test_binary(DEFAULT, NAND, 7);
  test_binary(DEFAULT, NAND, 31);
  test_binary(DEFAULT, NAND, 33);
}

TEST_F(TestBitVector, ne)
{
  test_binary(DEFAULT, NE, 1);
  test_binary(DEFAULT, NE, 7);
  test_binary(DEFAULT, NE, 31);
  test_binary(DEFAULT, NE, 33);
}

TEST_F(TestBitVector, or)
{
  test_binary(DEFAULT, OR, 1);
  test_binary(DEFAULT, OR, 7);
  test_binary(DEFAULT, OR, 31);
  test_binary(DEFAULT, OR, 33);
}

TEST_F(TestBitVector, nor)
{
  test_binary(DEFAULT, NOR, 1);
  test_binary(DEFAULT, NOR, 7);
  test_binary(DEFAULT, NOR, 31);
  test_binary(DEFAULT, NOR, 33);
}

TEST_F(TestBitVector, sdiv)
{
  test_binary_signed(DEFAULT, SDIV, 1);
  test_binary_signed(DEFAULT, SDIV, 7);
  test_binary_signed(DEFAULT, SDIV, 31);
  test_binary_signed(DEFAULT, SDIV, 33);
}

TEST_F(TestBitVector, sext)
{
  test_extend(DEFAULT, SEXT, 2);
  test_extend(DEFAULT, SEXT, 3);
  test_extend(DEFAULT, SEXT, 4);
  test_extend(DEFAULT, SEXT, 5);
  test_extend(DEFAULT, SEXT, 6);
  test_extend(DEFAULT, SEXT, 7);
  test_extend(DEFAULT, SEXT, 31);
  test_extend(DEFAULT, SEXT, 33);
}

TEST_F(TestBitVector, shl)
{
  test_binary(DEFAULT, SHL, 2);
  test_binary(DEFAULT, SHL, 8);
  test_binary(DEFAULT, SHL, 16);
  test_binary(DEFAULT, SHL, 32);
  test_shift(DEFAULT, SHL, true);
  test_shift(DEFAULT, SHL, false);
}

TEST_F(TestBitVector, shr)
{
  test_binary(DEFAULT, SHR, 2);
  test_binary(DEFAULT, SHR, 8);
  test_binary(DEFAULT, SHR, 16);
  test_binary(DEFAULT, SHR, 32);
  test_shift(DEFAULT, SHR, true);
  test_shift(DEFAULT, SHR, false);
}

TEST_F(TestBitVector, ashr)
{
  test_binary(DEFAULT, ASHR, 2);
  test_binary(DEFAULT, ASHR, 8);
  test_binary(DEFAULT, ASHR, 16);
  test_binary(DEFAULT, ASHR, 32);
  test_shift(DEFAULT, ASHR, false);
}

TEST_F(TestBitVector, slt)
{
  test_binary_signed(DEFAULT, SLT, 1);
  test_binary_signed(DEFAULT, SLT, 7);
  test_binary_signed(DEFAULT, SLT, 31);
  test_binary_signed(DEFAULT, SLT, 33);
}

TEST_F(TestBitVector, sle)
{
  test_binary_signed(DEFAULT, SLE, 1);
  test_binary_signed(DEFAULT, SLE, 7);
  test_binary_signed(DEFAULT, SLE, 31);
  test_binary_signed(DEFAULT, SLE, 33);
}

TEST_F(TestBitVector, sgt)
{
  test_binary_signed(DEFAULT, SGT, 1);
  test_binary_signed(DEFAULT, SGT, 7);
  test_binary_signed(DEFAULT, SGT, 31);
  test_binary_signed(DEFAULT, SGT, 33);
}

TEST_F(TestBitVector, sge)
{
  test_binary_signed(DEFAULT, SGE, 1);
  test_binary_signed(DEFAULT, SGE, 7);
  test_binary_signed(DEFAULT, SGE, 31);
  test_binary_signed(DEFAULT, SGE, 33);
}

TEST_F(TestBitVector, sub)
{
  test_binary(DEFAULT, SUB, 1);
  test_binary(DEFAULT, SUB, 7);
  test_binary(DEFAULT, SUB, 31);
  test_binary(DEFAULT, SUB, 33);
}

TEST_F(TestBitVector, srem)
{
  test_binary_signed(DEFAULT, SREM, 1);
  test_binary_signed(DEFAULT, SREM, 7);
  test_binary_signed(DEFAULT, SREM, 31);
  test_binary_signed(DEFAULT, SREM, 33);
}

TEST_F(TestBitVector, udiv)
{
  test_binary(DEFAULT, UDIV, 1);
  test_binary(DEFAULT, UDIV, 7);
  test_binary(DEFAULT, UDIV, 31);
  test_binary(DEFAULT, UDIV, 33);
}

TEST_F(TestBitVector, ult)
{
  test_binary(DEFAULT, ULT, 1);
  test_binary(DEFAULT, ULT, 7);
  test_binary(DEFAULT, ULT, 31);
  test_binary(DEFAULT, ULT, 33);
}

TEST_F(TestBitVector, ule)
{
  test_binary(DEFAULT, ULE, 1);
  test_binary(DEFAULT, ULE, 7);
  test_binary(DEFAULT, ULE, 31);
  test_binary(DEFAULT, ULE, 33);
}

TEST_F(TestBitVector, ugt)
{
  test_binary(DEFAULT, UGT, 1);
  test_binary(DEFAULT, UGT, 7);
  test_binary(DEFAULT, UGT, 31);
  test_binary(DEFAULT, UGT, 33);
}

TEST_F(TestBitVector, uge)
{
  test_binary(DEFAULT, UGE, 1);
  test_binary(DEFAULT, UGE, 7);
  test_binary(DEFAULT, UGE, 31);
  test_binary(DEFAULT, UGE, 33);
}

TEST_F(TestBitVector, urem)
{
  test_binary(DEFAULT, UREM, 1);
  test_binary(DEFAULT, UREM, 7);
  test_binary(DEFAULT, UREM, 31);
  test_binary(DEFAULT, UREM, 33);
}

TEST_F(TestBitVector, xor)
{
  test_binary(DEFAULT, XOR, 1);
  test_binary(DEFAULT, XOR, 7);
  test_binary(DEFAULT, XOR, 31);
  test_binary(DEFAULT, XOR, 33);
}

TEST_F(TestBitVector, xnor)
{
  test_binary(DEFAULT, XNOR, 1);
  test_binary(DEFAULT, XNOR, 7);
  test_binary(DEFAULT, XNOR, 31);
  test_binary(DEFAULT, XNOR, 33);
}

TEST_F(TestBitVector, zext)
{
  test_extend(DEFAULT, ZEXT, 2);
  test_extend(DEFAULT, ZEXT, 3);
  test_extend(DEFAULT, ZEXT, 4);
  test_extend(DEFAULT, ZEXT, 5);
  test_extend(DEFAULT, ZEXT, 6);
  test_extend(DEFAULT, ZEXT, 7);
  test_extend(DEFAULT, ZEXT, 31);
  test_extend(DEFAULT, ZEXT, 33);
}

/* -------------------------------------------------------------------------- */

TEST_F(TestBitVector, idec)
{
  test_unary(INPLACE_NOT_CHAINABLE, DEC, 1);
  test_unary(INPLACE_NOT_CHAINABLE, DEC, 7);
  test_unary(INPLACE_NOT_CHAINABLE, DEC, 31);
  test_unary(INPLACE_NOT_CHAINABLE, DEC, 33);
  test_unary(INPLACE_CHAINABLE, DEC, 1);
  test_unary(INPLACE_CHAINABLE, DEC, 7);
  test_unary(INPLACE_CHAINABLE, DEC, 31);
  test_unary(INPLACE_CHAINABLE, DEC, 33);
}

TEST_F(TestBitVector, iinc)
{
  test_unary(INPLACE_NOT_CHAINABLE, INC, 1);
  test_unary(INPLACE_NOT_CHAINABLE, INC, 7);
  test_unary(INPLACE_NOT_CHAINABLE, INC, 31);
  test_unary(INPLACE_NOT_CHAINABLE, INC, 33);
  test_unary(INPLACE_CHAINABLE, INC, 1);
  test_unary(INPLACE_CHAINABLE, INC, 7);
  test_unary(INPLACE_CHAINABLE, INC, 31);
  test_unary(INPLACE_CHAINABLE, INC, 33);
}

TEST_F(TestBitVector, ineg)
{
  test_unary(INPLACE_NOT_CHAINABLE, NEG, 1);
  test_unary(INPLACE_NOT_CHAINABLE, NEG, 7);
  test_unary(INPLACE_NOT_CHAINABLE, NEG, 31);
  test_unary(INPLACE_NOT_CHAINABLE, NEG, 33);
  test_unary(INPLACE_CHAINABLE, NEG, 1);
  test_unary(INPLACE_CHAINABLE, NEG, 7);
  test_unary(INPLACE_CHAINABLE, NEG, 31);
  test_unary(INPLACE_CHAINABLE, NEG, 33);
}

TEST_F(TestBitVector, inot)
{
  test_unary(INPLACE_NOT_CHAINABLE, NOT, 1);
  test_unary(INPLACE_NOT_CHAINABLE, NOT, 7);
  test_unary(INPLACE_NOT_CHAINABLE, NOT, 31);
  test_unary(INPLACE_NOT_CHAINABLE, NOT, 33);
  test_unary(INPLACE_CHAINABLE, NOT, 1);
  test_unary(INPLACE_CHAINABLE, NOT, 7);
  test_unary(INPLACE_CHAINABLE, NOT, 31);
  test_unary(INPLACE_CHAINABLE, NOT, 33);
}

TEST_F(TestBitVector, iredand)
{
  test_unary(INPLACE_NOT_CHAINABLE, REDAND, 1);
  test_unary(INPLACE_NOT_CHAINABLE, REDAND, 7);
  test_unary(INPLACE_NOT_CHAINABLE, REDAND, 31);
  test_unary(INPLACE_NOT_CHAINABLE, REDAND, 33);
  test_unary(INPLACE_CHAINABLE, REDAND, 1);
  test_unary(INPLACE_CHAINABLE, REDAND, 7);
  test_unary(INPLACE_CHAINABLE, REDAND, 31);
  test_unary(INPLACE_CHAINABLE, REDAND, 33);
}

TEST_F(TestBitVector, iredor)
{
  test_unary(INPLACE_NOT_CHAINABLE, REDOR, 1);
  test_unary(INPLACE_NOT_CHAINABLE, REDOR, 7);
  test_unary(INPLACE_NOT_CHAINABLE, REDOR, 31);
  test_unary(INPLACE_NOT_CHAINABLE, REDOR, 33);
  test_unary(INPLACE_CHAINABLE, REDOR, 1);
  test_unary(INPLACE_CHAINABLE, REDOR, 7);
  test_unary(INPLACE_CHAINABLE, REDOR, 31);
  test_unary(INPLACE_CHAINABLE, REDOR, 33);
}

TEST_F(TestBitVector, iadd)
{
  test_binary(INPLACE_NOT_CHAINABLE, ADD, 1);
  test_binary(INPLACE_NOT_CHAINABLE, ADD, 7);
  test_binary(INPLACE_NOT_CHAINABLE, ADD, 31);
  test_binary(INPLACE_NOT_CHAINABLE, ADD, 33);
  test_binary(INPLACE_CHAINABLE, ADD, 1);
  test_binary(INPLACE_CHAINABLE, ADD, 7);
  test_binary(INPLACE_CHAINABLE, ADD, 31);
  test_binary(INPLACE_CHAINABLE, ADD, 33);
}

TEST_F(TestBitVector, iand)
{
  test_binary(INPLACE_NOT_CHAINABLE, AND, 1);
  test_binary(INPLACE_NOT_CHAINABLE, AND, 7);
  test_binary(INPLACE_NOT_CHAINABLE, AND, 31);
  test_binary(INPLACE_NOT_CHAINABLE, AND, 33);
  test_binary(INPLACE_CHAINABLE, AND, 1);
  test_binary(INPLACE_CHAINABLE, AND, 7);
  test_binary(INPLACE_CHAINABLE, AND, 31);
  test_binary(INPLACE_CHAINABLE, AND, 33);
}

TEST_F(TestBitVector, iconcat)
{
  test_concat(INPLACE_NOT_CHAINABLE, 2);
  test_concat(INPLACE_NOT_CHAINABLE, 7);
  test_concat(INPLACE_NOT_CHAINABLE, 31);
  test_concat(INPLACE_NOT_CHAINABLE, 33);
  test_concat(INPLACE_NOT_CHAINABLE, 64);
}

TEST_F(TestBitVector, ieq)
{
  test_binary(INPLACE_NOT_CHAINABLE, EQ, 1);
  test_binary(INPLACE_NOT_CHAINABLE, EQ, 7);
  test_binary(INPLACE_NOT_CHAINABLE, EQ, 31);
  test_binary(INPLACE_NOT_CHAINABLE, EQ, 33);
  test_binary(INPLACE_CHAINABLE, EQ, 1);
  test_binary(INPLACE_CHAINABLE, EQ, 7);
  test_binary(INPLACE_CHAINABLE, EQ, 31);
  test_binary(INPLACE_CHAINABLE, EQ, 33);
}

TEST_F(TestBitVector, iextract)
{
  test_extract(INPLACE_NOT_CHAINABLE, 1);
  test_extract(INPLACE_NOT_CHAINABLE, 7);
  test_extract(INPLACE_NOT_CHAINABLE, 31);
  test_extract(INPLACE_NOT_CHAINABLE, 33);
}

TEST_F(TestBitVector, iimplies)
{
  test_binary(INPLACE_NOT_CHAINABLE, IMPLIES, 1);
  test_binary(INPLACE_CHAINABLE, IMPLIES, 1);
}

TEST_F(TestBitVector, iite)
{
  test_ite(INPLACE_NOT_CHAINABLE, 1);
  test_ite(INPLACE_NOT_CHAINABLE, 7);
  test_ite(INPLACE_NOT_CHAINABLE, 31);
  test_ite(INPLACE_NOT_CHAINABLE, 33);
}

TEST_F(TestBitVector, imodinv)
{
  test_ite(INPLACE_NOT_CHAINABLE, 1);
  test_ite(INPLACE_NOT_CHAINABLE, 7);
  test_ite(INPLACE_NOT_CHAINABLE, 31);
  test_ite(INPLACE_NOT_CHAINABLE, 33);
}

TEST_F(TestBitVector, imul)
{
  test_binary(INPLACE_NOT_CHAINABLE, MUL, 1);
  test_binary(INPLACE_NOT_CHAINABLE, MUL, 7);
  test_binary(INPLACE_NOT_CHAINABLE, MUL, 31);
  test_binary(INPLACE_NOT_CHAINABLE, MUL, 33);
}

TEST_F(TestBitVector, inand)
{
  test_binary(INPLACE_NOT_CHAINABLE, NAND, 1);
  test_binary(INPLACE_NOT_CHAINABLE, NAND, 7);
  test_binary(INPLACE_NOT_CHAINABLE, NAND, 31);
  test_binary(INPLACE_NOT_CHAINABLE, NAND, 33);
  test_binary(INPLACE_CHAINABLE, NAND, 1);
  test_binary(INPLACE_CHAINABLE, NAND, 7);
  test_binary(INPLACE_CHAINABLE, NAND, 31);
  test_binary(INPLACE_CHAINABLE, NAND, 33);
}

TEST_F(TestBitVector, ine)
{
  test_binary(INPLACE_NOT_CHAINABLE, NE, 1);
  test_binary(INPLACE_NOT_CHAINABLE, NE, 7);
  test_binary(INPLACE_NOT_CHAINABLE, NE, 31);
  test_binary(INPLACE_NOT_CHAINABLE, NE, 33);
  test_binary(INPLACE_CHAINABLE, NE, 1);
  test_binary(INPLACE_CHAINABLE, NE, 7);
  test_binary(INPLACE_CHAINABLE, NE, 31);
  test_binary(INPLACE_CHAINABLE, NE, 33);
}

TEST_F(TestBitVector, ior)
{
  test_binary(INPLACE_NOT_CHAINABLE, OR, 1);
  test_binary(INPLACE_NOT_CHAINABLE, OR, 7);
  test_binary(INPLACE_NOT_CHAINABLE, OR, 31);
  test_binary(INPLACE_NOT_CHAINABLE, OR, 33);
  test_binary(INPLACE_CHAINABLE, OR, 1);
  test_binary(INPLACE_CHAINABLE, OR, 7);
  test_binary(INPLACE_CHAINABLE, OR, 31);
  test_binary(INPLACE_CHAINABLE, OR, 33);
}

TEST_F(TestBitVector, inor)
{
  test_binary(INPLACE_NOT_CHAINABLE, NOR, 1);
  test_binary(INPLACE_NOT_CHAINABLE, NOR, 7);
  test_binary(INPLACE_NOT_CHAINABLE, NOR, 31);
  test_binary(INPLACE_NOT_CHAINABLE, NOR, 33);
  test_binary(INPLACE_CHAINABLE, NOR, 1);
  test_binary(INPLACE_CHAINABLE, NOR, 7);
  test_binary(INPLACE_CHAINABLE, NOR, 31);
  test_binary(INPLACE_CHAINABLE, NOR, 33);
}

TEST_F(TestBitVector, isdiv)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SDIV, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SDIV, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SDIV, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SDIV, 33);
}

TEST_F(TestBitVector, isext)
{
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 2);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 3);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 4);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 5);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 6);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 7);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 31);
  test_extend(INPLACE_NOT_CHAINABLE, SEXT, 33);
}

TEST_F(TestBitVector, ishl)
{
  test_binary(INPLACE_NOT_CHAINABLE, SHL, 2);
  test_binary(INPLACE_NOT_CHAINABLE, SHL, 8);
  test_binary(INPLACE_NOT_CHAINABLE, SHL, 16);
  test_binary(INPLACE_NOT_CHAINABLE, SHL, 32);
  test_shift(INPLACE_NOT_CHAINABLE, SHL, true);
  test_shift(INPLACE_NOT_CHAINABLE, SHL, false);
  test_binary(INPLACE_CHAINABLE, SHL, 2);
  test_binary(INPLACE_CHAINABLE, SHL, 8);
  test_binary(INPLACE_CHAINABLE, SHL, 16);
  test_binary(INPLACE_CHAINABLE, SHL, 32);
  test_shift(INPLACE_CHAINABLE, SHL, true);
  test_shift(INPLACE_CHAINABLE, SHL, false);
}

TEST_F(TestBitVector, ishr)
{
  test_binary(INPLACE_NOT_CHAINABLE, SHR, 2);
  test_binary(INPLACE_NOT_CHAINABLE, SHR, 8);
  test_binary(INPLACE_NOT_CHAINABLE, SHR, 16);
  test_binary(INPLACE_NOT_CHAINABLE, SHR, 32);
  test_shift(INPLACE_NOT_CHAINABLE, SHR, true);
  test_shift(INPLACE_NOT_CHAINABLE, SHR, false);
  test_binary(INPLACE_CHAINABLE, SHR, 2);
  test_binary(INPLACE_CHAINABLE, SHR, 8);
  test_binary(INPLACE_CHAINABLE, SHR, 16);
  test_binary(INPLACE_CHAINABLE, SHR, 32);
  test_shift(INPLACE_CHAINABLE, SHR, true);
  test_shift(INPLACE_CHAINABLE, SHR, false);
}

TEST_F(TestBitVector, iashr)
{
  test_binary(INPLACE_NOT_CHAINABLE, ASHR, 2);
  test_binary(INPLACE_NOT_CHAINABLE, ASHR, 8);
  test_binary(INPLACE_NOT_CHAINABLE, ASHR, 16);
  test_binary(INPLACE_NOT_CHAINABLE, ASHR, 32);
  test_shift(INPLACE_NOT_CHAINABLE, ASHR, false);
  test_binary(INPLACE_CHAINABLE, ASHR, 2);
  test_binary(INPLACE_CHAINABLE, ASHR, 8);
  test_binary(INPLACE_CHAINABLE, ASHR, 16);
  test_binary(INPLACE_CHAINABLE, ASHR, 32);
  test_shift(INPLACE_CHAINABLE, ASHR, false);
}

TEST_F(TestBitVector, islt)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLT, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLT, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLT, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLT, 33);
  test_binary_signed(INPLACE_CHAINABLE, SLT, 1);
  test_binary_signed(INPLACE_CHAINABLE, SLT, 7);
  test_binary_signed(INPLACE_CHAINABLE, SLT, 31);
  test_binary_signed(INPLACE_CHAINABLE, SLT, 33);
}

TEST_F(TestBitVector, isle)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLE, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLE, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLE, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SLE, 33);
  test_binary_signed(INPLACE_CHAINABLE, SLE, 1);
  test_binary_signed(INPLACE_CHAINABLE, SLE, 7);
  test_binary_signed(INPLACE_CHAINABLE, SLE, 31);
  test_binary_signed(INPLACE_CHAINABLE, SLE, 33);
}

TEST_F(TestBitVector, isgt)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGT, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGT, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGT, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGT, 33);
  test_binary_signed(INPLACE_CHAINABLE, SGT, 1);
  test_binary_signed(INPLACE_CHAINABLE, SGT, 7);
  test_binary_signed(INPLACE_CHAINABLE, SGT, 31);
  test_binary_signed(INPLACE_CHAINABLE, SGT, 33);
}

TEST_F(TestBitVector, isge)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGE, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGE, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGE, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SGE, 33);
  test_binary_signed(INPLACE_CHAINABLE, SGE, 1);
  test_binary_signed(INPLACE_CHAINABLE, SGE, 7);
  test_binary_signed(INPLACE_CHAINABLE, SGE, 31);
  test_binary_signed(INPLACE_CHAINABLE, SGE, 33);
}

TEST_F(TestBitVector, isub)
{
  test_binary(INPLACE_NOT_CHAINABLE, SUB, 1);
  test_binary(INPLACE_NOT_CHAINABLE, SUB, 7);
  test_binary(INPLACE_NOT_CHAINABLE, SUB, 31);
  test_binary(INPLACE_NOT_CHAINABLE, SUB, 33);
  test_binary(INPLACE_CHAINABLE, SUB, 1);
  test_binary(INPLACE_CHAINABLE, SUB, 7);
  test_binary(INPLACE_CHAINABLE, SUB, 31);
  test_binary(INPLACE_CHAINABLE, SUB, 33);
}

TEST_F(TestBitVector, isrem)
{
  test_binary_signed(INPLACE_NOT_CHAINABLE, SREM, 1);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SREM, 7);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SREM, 31);
  test_binary_signed(INPLACE_NOT_CHAINABLE, SREM, 33);
}

TEST_F(TestBitVector, iudiv)
{
  test_binary(INPLACE_NOT_CHAINABLE, UDIV, 1);
  test_binary(INPLACE_NOT_CHAINABLE, UDIV, 7);
  test_binary(INPLACE_NOT_CHAINABLE, UDIV, 31);
  test_binary(INPLACE_NOT_CHAINABLE, UDIV, 33);
}

TEST_F(TestBitVector, iult)
{
  test_binary(INPLACE_NOT_CHAINABLE, ULT, 1);
  test_binary(INPLACE_NOT_CHAINABLE, ULT, 7);
  test_binary(INPLACE_NOT_CHAINABLE, ULT, 31);
  test_binary(INPLACE_NOT_CHAINABLE, ULT, 33);
  test_binary(INPLACE_CHAINABLE, ULT, 1);
  test_binary(INPLACE_CHAINABLE, ULT, 7);
  test_binary(INPLACE_CHAINABLE, ULT, 31);
  test_binary(INPLACE_CHAINABLE, ULT, 33);
}

TEST_F(TestBitVector, iule)
{
  test_binary(INPLACE_NOT_CHAINABLE, ULE, 1);
  test_binary(INPLACE_NOT_CHAINABLE, ULE, 7);
  test_binary(INPLACE_NOT_CHAINABLE, ULE, 31);
  test_binary(INPLACE_NOT_CHAINABLE, ULE, 33);
  test_binary(INPLACE_CHAINABLE, ULE, 1);
  test_binary(INPLACE_CHAINABLE, ULE, 7);
  test_binary(INPLACE_CHAINABLE, ULE, 31);
  test_binary(INPLACE_CHAINABLE, ULE, 33);
}

TEST_F(TestBitVector, iugt)
{
  test_binary(INPLACE_NOT_CHAINABLE, UGT, 1);
  test_binary(INPLACE_NOT_CHAINABLE, UGT, 7);
  test_binary(INPLACE_NOT_CHAINABLE, UGT, 31);
  test_binary(INPLACE_NOT_CHAINABLE, UGT, 33);
  test_binary(INPLACE_CHAINABLE, UGT, 1);
  test_binary(INPLACE_CHAINABLE, UGT, 7);
  test_binary(INPLACE_CHAINABLE, UGT, 31);
  test_binary(INPLACE_CHAINABLE, UGT, 33);
}

TEST_F(TestBitVector, iuge)
{
  test_binary(INPLACE_NOT_CHAINABLE, UGE, 1);
  test_binary(INPLACE_NOT_CHAINABLE, UGE, 7);
  test_binary(INPLACE_NOT_CHAINABLE, UGE, 31);
  test_binary(INPLACE_NOT_CHAINABLE, UGE, 33);
  test_binary(INPLACE_CHAINABLE, UGE, 1);
  test_binary(INPLACE_CHAINABLE, UGE, 7);
  test_binary(INPLACE_CHAINABLE, UGE, 31);
  test_binary(INPLACE_CHAINABLE, UGE, 33);
}

TEST_F(TestBitVector, iurem)
{
  test_binary(INPLACE_NOT_CHAINABLE, UREM, 1);
  test_binary(INPLACE_NOT_CHAINABLE, UREM, 7);
  test_binary(INPLACE_NOT_CHAINABLE, UREM, 31);
  test_binary(INPLACE_NOT_CHAINABLE, UREM, 33);
}
TEST_F(TestBitVector, ixor)
{
  test_binary(INPLACE_NOT_CHAINABLE, XOR, 1);
  test_binary(INPLACE_NOT_CHAINABLE, XOR, 7);
  test_binary(INPLACE_NOT_CHAINABLE, XOR, 31);
  test_binary(INPLACE_NOT_CHAINABLE, XOR, 33);
  test_binary(INPLACE_CHAINABLE, XOR, 1);
  test_binary(INPLACE_CHAINABLE, XOR, 7);
  test_binary(INPLACE_CHAINABLE, XOR, 31);
  test_binary(INPLACE_CHAINABLE, XOR, 33);
}

TEST_F(TestBitVector, ixnor)
{
  test_binary(INPLACE_NOT_CHAINABLE, XNOR, 1);
  test_binary(INPLACE_NOT_CHAINABLE, XNOR, 7);
  test_binary(INPLACE_NOT_CHAINABLE, XNOR, 31);
  test_binary(INPLACE_NOT_CHAINABLE, XNOR, 33);
  test_binary(INPLACE_CHAINABLE, XNOR, 1);
  test_binary(INPLACE_CHAINABLE, XNOR, 7);
  test_binary(INPLACE_CHAINABLE, XNOR, 31);
  test_binary(INPLACE_CHAINABLE, XNOR, 33);
}

TEST_F(TestBitVector, izext)
{
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 2);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 3);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 4);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 5);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 6);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 7);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 31);
  test_extend(INPLACE_NOT_CHAINABLE, ZEXT, 33);
}

/* -------------------------------------------------------------------------- */

TEST_F(TestBitVector, add32)
{
  BitVector res;
  BitVector a0(32, *d_rng);
  BitVector a1(32, *d_rng);
  for (uint32_t i = 0; i < 10000000; ++i)
  {
    res = a0.bvadd(a1);
  }
}

TEST_F(TestBitVector, iadd32)
{
  BitVector res(32);
  BitVector a0(32, *d_rng);
  BitVector a1(32, *d_rng);
  for (uint32_t i = 0; i < 10000000; ++i)
  {
    res.ibvadd(a0, a1);
  }
}

/* -------------------------------------------------------------------------- */

TEST_F(TestBitVector, udivurem)
{
  test_udivurem(1);
  test_udivurem(7);
  test_udivurem(31);
  test_udivurem(33);
}

/* -------------------------------------------------------------------------- */

}  // namespace test
}  // namespace bzlals