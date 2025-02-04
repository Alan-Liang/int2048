#ifndef SJTU_BIGINTEGER_H
#define SJTU_BIGINTEGER_H

#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <complex>

// Permitted by @Sakits
#include <cstdint>

#ifdef ONLINE_JUDGE
// using `sizeof` here to suppress annoying warnings on unused variables, etc.
#define assert(...) sizeof(__VA_ARGS__)
#else
#include <cassert>
#endif

namespace sjtu {
class int2048 {
 private:
  using SegType = std::uint_fast32_t;
  static constexpr SegType EXP10[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
  static constexpr int SEG_LENGTH = 9;
  static constexpr SegType SEG_MAX = EXP10[SEG_LENGTH];
  using TmpType = std::int_fast64_t;
  /// Little-endian.
  std::vector<SegType> segments_;
  /// True if negative.
  bool signbit_ = false;

  static SegType ninesComplement_ (const SegType &number) { return SEG_MAX - number - 1; }

  /// Strip leading zeroes.
  void normalize_ () {
    while (segments_.back() == 0) segments_.pop_back();
  }
  /// Adds values of this and that in place, disregarding sign bit.
  int2048 &addValue_ (const int2048 &that) {
    const int szThis = segments_.size();
    const int szThat = that.segments_.size();
    const int szMax = szThat > szThis ? szThat : szThis;
    TmpType carry = 0;
    for (int i = 0; i < szMax; ++i) {
      const TmpType segThis = i < szThis ? segments_[i] : 0;
      const TmpType segThat = i < szThat ? that.segments_[i] : 0;
      const TmpType sum = segThis + segThat + carry;
      const SegType sumOut = sum % SEG_MAX;
      carry = sum / SEG_MAX;
      if (i < szThis) {
        segments_[i] = sumOut;
      } else {
        segments_.push_back(sumOut);
      }
    }
    if (carry != 0) segments_.push_back(carry);
    return *this;
  }
  /// Gets the segment at index with optional left shift of SEG_MAX with bound checks.
  const SegType seg_ (const int &i, const bool &lshift = false) const {
    if (lshift) {
      if (i == 0) return 0;
      if (i - 1 >= segments_.size()) return 0;
      return segments_[i - 1];
    }
    if (i >= segments_.size()) return 0;
    return segments_[i];
  }
  /// Subtracts that from this in place, with optional left shift of that by SEG_MAX, disregarding sign bit.
  int2048 &subValue_ (const int2048 &that, bool lshift = false) {
    const int szThis = segments_.size();
    const int szThat = that.segments_.size() + (lshift ? 1 : 0);
    const int szMax = szThat > szThis ? szThat : szThis;
    TmpType borrow = 0;
    for (int i = 0; i < szMax; ++i) {
      const TmpType segThis = i < szThis ? segments_[i] : 0;
      const TmpType segThat = i < szThat ? that.seg_(i, lshift) : 0;
      TmpType difference = segThis - segThat - borrow;
      if (difference < 0) {
        borrow = 1;
        difference += SEG_MAX;
      } else {
        borrow = 0;
      }
      if (i < szThis) {
        segments_[i] = difference;
      } else {
        segments_.push_back(difference);
      }
    }
    if (borrow) {
      /* this happens when abs(that) > abs(this).
           abs(a - b) = b - a
         = -(SEG_MAX ** size + a - b) + SEG_MAX ** size
         = (SEG_MAX ** size - 1) - (SEG_MAX ** size + a - b) + 1.
         SEG_MAX ** size + a - b is what we currently have in this.
         SEG_MAX ** size - 1 is a series of 9s, so ((SEG_MAX ** size - 1) - this) is the nine's complement of this.
         using nine's complement here because it never borrows.
         we need to add one to the complement to get the result. */
      signbit_ = !signbit_;
      for (auto &seg : segments_) {
        seg = ninesComplement_(seg);
      }
      for (auto &seg : segments_) {
        if (seg < SEG_MAX - 1) {
          ++seg;
          break;
        }
        seg = 0;
      }
    }
    normalize_();
    return *this;
  }
  /// Resets the sign bit and the segments to their initial states.
  void reset_ () {
    signbit_ = false;
    segments_.clear();
  }
  /// Resets this to number, returns this.
  int2048 &resetTo_ (const SegType &number) {
    reset_();
    segments_.push_back(number);
    return *this;
  }

  /// True if this is 0.
  bool isNull_ () const {
    return segments_.size() == 1 && segments_[0] == static_cast<SegType>(0);
  }

 public:
  int2048 () {
    segments_.push_back(0);
  }
  int2048 (long long number) : signbit_(number < 0) {
    /* do not use `-number` here, because negating LLONG_MIN is undefined behavior.
       See: https://github.com/python/cpython/blob/4c792f39e688b11c7c19e411ed4f76a7baa44638/Objects/longobject.c#L191-L192
       See: https://acm.sjtu.app/t/145 */
    unsigned long long current = number > 0 ? number : 0ULL - number;
    while (current > 0) {
      segments_.push_back(current % SEG_MAX);
      current /= SEG_MAX;
    }
    if (segments_.size() == 0) segments_.push_back(0);
  }
  int2048 (const std::string &string) {
    read(string);
  }
  int2048 (const int2048 &that) {
    *this = that;
  }
  int2048 (int2048 &&that) {
    // TODO: test if it works.
    segments_ = std::move(that.segments_);
    signbit_ = that.signbit_;
  }

  void read (const std::string &string) {
    reset_();
    // this is not IEEE 754. There are no negative zeroes.
    if (string == "-0" || string == "" || string == "0") {
      segments_.push_back(0);
      return;
    }
    int length = string.length();
    if (string[0] == '-') {
      --length;
      signbit_ = true;
    }
    int size = length / SEG_LENGTH + (length % SEG_LENGTH > 0 ? 1 : 0);
    segments_.reserve(size);
    SegType current = 0;
    for (int i = string.length() - 1; i >= 0; --i) {
      if (string[i] == '-') continue;
      const int num = string[i] - '0';
      const int offset = string.length() - 1 - i;
      const int segId = offset / SEG_LENGTH;
      const int pos = offset % SEG_LENGTH;
      current += num * EXP10[pos];
      if (pos == SEG_LENGTH - 1) {
        segments_.push_back(current);
        current = 0;
      }
    }
    if (current != 0) segments_.push_back(current);
  }
  void print () const {
    std::cout << *this;
  }

  int2048 &add (const int2048 &that) {
    return signbit_ == that.signbit_ ? addValue_(that) : subValue_(that);
  }
  friend int2048 &add (const int2048 &a, const int2048 &b) {
    return a + b;
  }

  int2048 &minus (const int2048 &that) {
    return signbit_ != that.signbit_ ? addValue_(that) : subValue_(that);
  }
  friend int2048 &minus (const int2048 &minuend, const int2048 &subtrahend) {
    return minuend - subtrahend;
  }

  int2048 &operator= (const int2048 &that) {
    segments_ = that.segments_;
    signbit_ = that.signbit_;
    return *this;
  }

  int2048 &operator+= (const int2048 &that) {
    return add(that);
  }
  friend int2048 &operator+ (const int2048 &a, const int2048 &b) {
    // TODO: are there ways to implement this without using `new`, `malloc`, or their relatives?
    return (new int2048(a))->add(b);
  }

  int2048 &operator-= (const int2048 &that) {
    return minus(that);
  }
  friend int2048 &operator- (const int2048 &minuend, const int2048 &subtrahend) {
    return (new int2048(minuend))->minus(subtrahend);
  }

  int2048 &operator*= (const int2048 &that) {
    // https://treskal.com/s/masters-thesis.pdf, Algorithm 2.3. Retrieved 2021/11/12.
    if (isNull_()) return *this;
    if (that.isNull_()) return resetTo_(0);
    if (that.signbit_) signbit_ = !signbit_;
    int2048 thatSnapshot = that;
    std::vector<SegType> result;
    const int szThis = segments_.size();
    const int szThat = that.segments_.size();
    const int szResult = szThis + szThat + 1;
    result.reserve(szResult);
    for (int i = 0; i < szResult; ++i) result.push_back(0);
    for (int i = 0; i < szThis; ++i) {
      TmpType q = 0;
      for (int j = 0; j < szThat; ++j) {
        TmpType tmp;
        tmp = static_cast<TmpType>(result[i + j]) + q + static_cast<TmpType>(segments_[i]) * static_cast<TmpType>(that.segments_[j]);
        result[i + j] = tmp % SEG_MAX;
        q = tmp / SEG_MAX;
      }
      result[i + szThat] = q;
    };
    segments_ = std::move(result);
    normalize_();
    segments_.shrink_to_fit();
    return *this;
  }
  friend int2048 &operator* (const int2048 &a, const int2048 &b) {
    return *(new int2048(a)) *= b;
  }

 private:
  template <typename T>
  struct DivisionResult_ {
    T quotient;
    T remainder;
  };
  static DivisionResult_<int2048> divSubroutine_ (const int2048 &divisor, const int2048 &dividend) {
    // https://treskal.com/s/masters-thesis.pdf, Algorithm 3.1. Retrieved 2021/11/12.
    const int szDivisor = divisor.segments_.size();
    const int szDividend = dividend.segments_.size();
    assert(szDivisor <= szDividend + 1);
    assert(dividend.segments_.back() >= SEG_MAX / 2);
    if (szDivisor == 1) {
      if (szDividend > 1) return { 0, dividend };
      return { divisor.segments_[0] / dividend.segments_[0], divisor.segments_[0] % dividend.segments_[0] };
    }
    const bool shouldSub = szDivisor == szDividend + 1 && divisor.segments_.back() >= dividend.segments_.back();
    if (shouldSub) {
      int2048 mutDivisor = divisor;
      mutDivisor.subValue_(dividend, /* lshift */ true);
      auto result = divSubroutine_(mutDivisor, dividend);
      result.quotient += SEG_MAX;
      return result;
    }
    TmpType q = static_cast<TmpType>(SEG_MAX) * static_cast<TmpType>(divisor.seg_(szDividend)) + static_cast<TmpType>(divisor.seg_(szDividend - 1));
    q /= static_cast<TmpType>(dividend.segments_.back());
    int2048 t = dividend;
    t *= q;
    while (t > divisor) {
      --q;
      t -= dividend;
    }
    DivisionResult_<int2048> res { q, divisor };
    res.remainder -= t;
    return res;
  }
  /// Right shifts by SEG_MAX ** cntSegments, and returns quotient and remainder.
  DivisionResult_<int2048> rshift_ (int cntSegments) const {
    DivisionResult_<int2048> res;
    res.remainder.reset_();
    res.quotient.reset_();
    res.remainder.segments_.reserve(cntSegments);
    res.quotient.segments_.reserve(segments_.size() - cntSegments);
    auto it = segments_.begin();
    for (int i = 0; i < cntSegments; ++i) {
      res.remainder.segments_.push_back(*it);
      ++it;
    }
    for (; it != segments_.end(); ++it) res.quotient.segments_.push_back(*it);
    // no need to normalize quotient here.
    res.remainder.normalize_();
    return res;
  }
 public:
  int2048 &operator/= (const int2048 &that) {
    // https://treskal.com/s/masters-thesis.pdf, Algorithm 3.2. Retrieved 2021/11/12.
    assert(!that.isNull_());
    if (isNull_()) return *this;
    assert(!signbit_ && !that.signbit_);
    if (*this < that) return resetTo_(0);
    const int szThis = segments_.size();
    const int szThat = that.segments_.size();
    if (that.segments_.back() < SEG_MAX / 2) {
      TmpType mul = SEG_MAX / 2 / that.segments_.back() + 1;
      int2048 newDividend = that;
      *this *= mul;
      newDividend *= mul;
      return *this /= newDividend;
    }
    if (szThis == szThat) return resetTo_(1);
    if (szThis == szThat + 1) return *this = std::move(divSubroutine_(*this, that).quotient);
    auto shifted = rshift_(szThis - szThat - 1);
    auto higher = divSubroutine_(shifted.quotient, that);
    int2048 &lowerDivisor = shifted.remainder;
    lowerDivisor.segments_.reserve(szThis - szThat - 1 + higher.remainder.segments_.size());
    while (lowerDivisor.segments_.size() < szThis - szThat - 1) lowerDivisor.segments_.push_back(0);
    for (const auto &seg : higher.remainder.segments_) lowerDivisor.segments_.push_back(seg);
    lowerDivisor /= that;
    *this = lowerDivisor;
    if (higher.quotient.isNull_()) return *this;
    segments_.reserve(szThis - szThat - 1 + higher.quotient.segments_.size());
    while (segments_.size() < szThis - szThat - 1) segments_.push_back(0);
    for (const auto &seg : higher.quotient.segments_) segments_.push_back(seg);
    return *this;
  }
  friend int2048 &operator/ (const int2048 &a, const int2048 &b) {
    return *(new int2048(a)) /= b;
  }

  friend std::istream &operator>> (std::istream &stream, int2048 &number) {
    std::string str;
    stream >> str;
    number.read(str);
    return stream;
  }
  friend std::ostream &operator<< (std::ostream &stream, const int2048 &number) {
    if (number.signbit_ && (number.segments_.size() > 1 || number.segments_[0] > 0)) stream << '-';
    bool firstSegment = true;
    for (auto it = number.segments_.rbegin(); it != number.segments_.rend(); ++it) {
      const auto &seg = *it;
      if (firstSegment) {
        firstSegment = false;
        stream << seg;
      } else {
        char str[SEG_LENGTH + 1];
        str[SEG_LENGTH] = '\0';
        SegType current = seg;
        for (int i = SEG_LENGTH - 1; i >= 0; --i) {
          str[i] = (current % 10) + '0';
          current /= 10;
        }
        stream << str;
      }
    }
    return stream;
  }

 private:
  /// -1 if lhs < rhs, 0 if lhs = rhs, 1 if lhs > rhs; ignores sign bit. Requires lhs and rhs be not null.
  static int cmpValue_ (const int2048 &lhs, const int2048 &rhs) {
    const int szLhs = lhs.segments_.size();
    const int szRhs = rhs.segments_.size();
    if (szLhs != szRhs) return cmp_(szLhs, szRhs);
    for (int i = szLhs - 1; i >= 0; --i) {
      int sgn = cmp_(lhs.segments_[i], rhs.segments_[i]);
      if (sgn != 0) return sgn;
    }
    return 0;
  }
  /// -1 if lhs < rhs, 0 if lhs = rhs, 1 if lhs > rhs
  static int cmp_ (const SegType &lhs, const SegType &rhs) {
    return lhs == rhs ? 0 : lhs < rhs ? -1 : 1;
  }
  /// -1 if lhs < rhs, 0 if lhs = rhs, 1 if lhs > rhs
  static int cmp_ (const int2048 &lhs, const int2048 &rhs) {
    if (lhs.isNull_()) return rhs.isNull_() ? 0 : rhs.signbit_ ? 1 : -1;
    if (rhs.isNull_() || lhs.signbit_ != rhs.signbit_) return lhs.signbit_ ? -1 : 1;
    return lhs.signbit_ ? -cmpValue_(lhs, rhs) : cmpValue_(lhs, rhs);
  }

 public:
  inline friend bool operator== (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) == 0; }
  inline friend bool operator!= (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) != 0; }
  inline friend bool operator<  (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) <  0; }
  inline friend bool operator>  (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) >  0; }
  inline friend bool operator<= (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) <= 0; }
  inline friend bool operator>= (const int2048 &lhs, const int2048 &rhs) { return cmp_(lhs, rhs) >= 0; }
};
} // namespace sjtu

#endif
