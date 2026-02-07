
// Copyright (c) 2013-2014 Sandstorm Development Group, Inc. and contributors
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "export-kj-async.h"
#include "common.h"
#include <cstdint>

KJ_BEGIN_HEADER

struct sockaddr;

namespace kj {

class KJ_ASYNC_CLASS CidrRange {
public:
  KJ_ASYNC_API CidrRange(StringPtr pattern);

  static CidrRange KJ_ASYNC_API inet4(ArrayPtr<const byte> bits, uint bitCount);
  static CidrRange KJ_ASYNC_API inet6(
      ArrayPtr<const uint16_t> prefix, ArrayPtr<const uint16_t> suffix, uint bitCount);
  // Zeros are inserted between `prefix` and `suffix` to extend the address to 128 bits.

  uint KJ_ASYNC_API getSpecificity() const { return bitCount; }

  bool KJ_ASYNC_API matches(const struct sockaddr* addr) const;
  bool KJ_ASYNC_API matchesFamily(int family) const;

  String KJ_ASYNC_API toString() const;

private:
  int family;
  byte bits[16];
  uint bitCount;    // how many bits in `bits` need to match

  CidrRange(int family, ArrayPtr<const byte> bits, uint bitCount);

  void zeroIrrelevantBits();
};

}  // namespace kj

KJ_END_HEADER
