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
    int2048 &subValue_ (const int2048 &that) {
      const int szThis = segments_.size();
      const int szThat = that.segments_.size();
      const int szMax = szThat > szThis ? szThat : szThis;
      TmpType take = 0;
      for (int i = 0; i < szMax; ++i) {
        const TmpType segThis = i < szThis ? segments_[i] : 0;
        const TmpType segThat = i < szThat ? that.segments_[i] : 0;
        TmpType difference = segThis - segThat - take;
        if (difference < 0) {
          take = 1;
          difference += SEG_MAX;
        } else {
          take = 0;
        }
        if (i < szThis) {
          segments_[i] = difference;
        } else {
          segments_.push_back(difference);
        }
      }
      if (take) {
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
      if (segments_.back() == 0) segments_.pop_back();
      return *this;
    }
    void reset_ () {
      signbit_ = false;
      segments_.clear();
    }
   public:
    int2048 () {
      segments_.push_back(0);
    }
    int2048 (long long number) : signbit_(number < 0) {
      if (number < 0) number = -number;
      TmpType current = number;
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

    void read (const std::string &string) {
      reset_();
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
    friend int2048 &minus (const int2048 &a, const int2048 &b) {
      return a - b;
    }

    // =================================== Integer2 ===================================

    int2048 &operator= (const int2048 &that) {
      segments_ = that.segments_;
      signbit_ = that.signbit_;
      return *this;
    }

    int2048 &operator+= (const int2048 &that) {
      return add(that);
    }
    friend int2048 &operator+ (const int2048 &a, const int2048 &b) {
      return (new int2048(a))->add(b);
    }

    int2048 &operator-= (const int2048 &that) {
      return minus(that);
    }
    friend int2048 &operator- (const int2048 &a, const int2048 &b) {
      return (new int2048(a))->minus(b);
    }

    int2048 &operator*= (const int2048 &that) {
      // TODO
      return *this;
    }
    friend int2048 &operator* (const int2048 &a, const int2048 &b) {
      return *(new int2048(a)) *= b;
    }

    int2048 &operator/= (const int2048 &that) {
      // TODO
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

    inline friend bool operator== (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
    inline friend bool operator!= (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
    inline friend bool operator< (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
    inline friend bool operator> (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
    inline friend bool operator<= (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
    inline friend bool operator>= (const int2048 &, const int2048 &) {
      // TODO
      return true;
    }
  };
}

#endif
