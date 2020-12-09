#ifndef BZLALS__BITVECTOR_DOMAIN_H
#define BZLALS__BITVECTOR_DOMAIN_H

#include "bitvector.h"

namespace bzlals {

class BitVectorDomain
{
 public:
  /** Construct a bit-vector domain of given size. */
  BitVectorDomain(uint32_t size);
  /** Construct a bit-vector domain ranging from 'lo' to 'hi'. */
  BitVectorDomain(const BitVector &lo, const BitVector &hi);
  /** Construct a bit-vector domain from a 3-valued string representation. */
  BitVectorDomain(const std::string &value);
  /** Construct a fixed bit-vector domain with lo = 'bv' and hi = 'bv'. */
  BitVectorDomain(const BitVector &bv);
  /** Construct a fixed bit-vector domain of given size from a uint value. */
  BitVectorDomain(uint32_t size, uint64_t value);
  /** Copy constructor. */
  BitVectorDomain(const BitVectorDomain &other);
  /** Destructor. */
  ~BitVectorDomain();

  /** Return the size of this bit-vector. */
  uint32_t get_size() const;

  /** Return true if this bit-vector domain is valid, i.e., ~lo | hi == ones. */
  bool is_valid() const;

  /** Return true if this bit-vector domain is fixed, i.e., lo == hi. */
  bool is_fixed() const;
  /**
   * Return true if this bit-vector domain has fixed bits, i.e., bits that are
   * assigned to the same value in both 'hi' and 'lo'.
   */
  bool has_fixed_bits() const;
  /** Return true if bit at given index is fixed. */
  bool is_fixed_bit(uint32_t idx);
  /** Return true if bit at given index is fixed and true. */
  bool is_fixed_bit_true(uint32_t idx);
  /** Return true if bit at given index is fixed and false. */
  bool is_fixed_bit_false(uint32_t idx);

  /** Fix bit at given index to given value. */
  void fix_bit(uint32_t idx, bool value);

  /**
   * Return true if fixed bits of this bit-vector domain are consistent with
   * the corresponding bits of the given bit-vector. That is, if a bit is fixed
   * to a value, it must have the same value in the bit-vector.
   */
  bool match_fixed_bits(const BitVector &bv) const;

  /** Equality comparison operator. */
  bool operator==(const BitVectorDomain &other) const;

  /**
   * Create a bit-vector domain that represents a bit-wise not of this domain.
   */
  BitVectorDomain bvnot() const;
  /**
   * Create a bit-vector domain that represents a logial left shift of this
   * domain by the shift value represented as bit-vector 'bv'.
   */
  BitVectorDomain bvshl(const BitVector &shift) const;

  /**
   * Extract a bit range from this bit-vector domain.
   * idx_hi: The upper bit-index of the range (inclusive).
   * idx_lo: The lower bit-index of the range (inclusive).
   */
  BitVectorDomain bvextract(uint32_t idx_hi, uint32_t idx_lo) const;

  /**
   * Return a string representation of this bit-vector domain.
   * Unset bits are represented as 'x', invalid bits are represented as 'i'.
   */
  std::string to_string() const;

 private:
  /**
   * The lower bound of this bit-vector domain.
   * Bits that are not fixed are set to 0. If a bit is '1' in 'lo' and '0' in
   * 'hi', the domain is invalid.
   */
  BitVector d_lo;
  /**
   * The upper bound of this bit-vector domain.
   * Bits that are not fixed are set to 1. If a bit is '1' in 'lo' and '0' in
   * 'hi', the domain is invalid.
   */
  BitVector d_hi;
};

}  // namespace bzlals

#endif