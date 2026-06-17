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

#include "export-capnp.h"
#include <kj/common.h>
#include <kj/string.h>
#include "common.h"
#include <string.h>

CAPNP_BEGIN_HEADER

namespace capnp {

struct CAPNP_CLASS Data {
  Data() = delete;
  class Reader;
  class Builder;
  class Pipeline {};
};

struct CAPNP_CLASS Text {
  Text() = delete;
  class Reader;
  class Builder;
  class Pipeline {};
};

class CAPNP_CLASS Data::Reader: public kj::ArrayPtr<const byte> {
  // Points to a blob of bytes.  The usual Reader rules apply -- Data::Reader behaves like a simple
  // pointer which does not own its target, can be passed by value, etc.

public:
  typedef Data Reads;

  CAPNP_API Reader() = default;
  CAPNP_API inline Reader(decltype(nullptr)): ArrayPtr<const byte>(nullptr) {}
  CAPNP_API inline Reader(const byte* value, size_t size): ArrayPtr<const byte>(value, size) {}
  CAPNP_API inline Reader(const kj::Array<const byte>& value): ArrayPtr<const byte>(value) {}
  CAPNP_API inline Reader(const ArrayPtr<const byte>& value): ArrayPtr<const byte>(value) {}
  CAPNP_API inline Reader(const kj::Array<byte>& value): ArrayPtr<const byte>(value) {}
  CAPNP_API inline Reader(const ArrayPtr<byte>& value): ArrayPtr<const byte>(value) {}
};

class CAPNP_CLASS Text::Reader: public kj::StringPtr {
  // Like Data::Reader, but points at NUL-terminated UTF-8 text.  The NUL terminator is not counted
  // in the size but must be present immediately after the last byte.
  //
  // Text::Reader's interface contract is that its data MUST be NUL-terminated.  The producer of
  // the Text::Reader must guarantee this, so that the consumer need not check.  The data SHOULD
  // also be valid UTF-8, but this is NOT guaranteed -- the consumer must verify if it cares.

public:
  typedef Text Reads;

  CAPNP_API Reader() = default;
  CAPNP_API inline Reader(decltype(nullptr)): StringPtr(nullptr) {}
  CAPNP_API inline Reader(const char* value): StringPtr(value) {}
  CAPNP_API inline Reader(const char* value, size_t size): StringPtr(value, size) {}
  CAPNP_API inline Reader(const kj::String& value): StringPtr(value) {}
  CAPNP_API inline Reader(const StringPtr& value): StringPtr(value) {}

#if KJ_COMPILER_SUPPORTS_STL_STRING_INTEROP
  template <
    typename T,
    typename = kj::EnableIf<kj::canConvert<decltype(kj::instance<T>().c_str()), const char*>()>>
  inline Reader(const T& t): StringPtr(t) {}
  // Allow implicit conversion from any class that has a c_str() method (namely, std::string).
  // We use a template trick to detect std::string in order to avoid including the header for
  // those who don't want it.
#endif
};

class CAPNP_CLASS Data::Builder: public kj::ArrayPtr<byte> {
  // Like Data::Reader except the pointers aren't const.

public:
  typedef Data Builds;

  CAPNP_API Builder() = default;
  CAPNP_API inline Builder(decltype(nullptr)): ArrayPtr<byte>(nullptr) {}
  CAPNP_API inline Builder(byte* value, size_t size): ArrayPtr<byte>(value, size) {}
  CAPNP_API inline Builder(kj::Array<byte>& value): ArrayPtr<byte>(value) {}
  CAPNP_API inline Builder(ArrayPtr<byte> value): ArrayPtr<byte>(value) {}

  CAPNP_API inline Data::Reader asReader() const {
    return Data::Reader(kj::implicitCast<const kj::ArrayPtr<byte>&>(*this));
  }
  CAPNP_API inline operator Reader() const { return asReader(); }
};

class CAPNP_CLASS Text::Builder: public kj::DisallowConstCopy {
  // Basically identical to kj::StringPtr, except that the contents are non-const.

public:
  CAPNP_API inline Builder(): content(nulstr, 1) {}
  CAPNP_API inline Builder(decltype(nullptr)): content(nulstr, 1) {}
  CAPNP_API inline Builder(char* value): content(value, strlen(value) + 1) {}
  CAPNP_API inline Builder(char* value, size_t size): content(value, size + 1) {
    KJ_IREQUIRE(value[size] == '\0', "StringPtr must be NUL-terminated.");
  }

  CAPNP_API inline Reader asReader() const { return Reader(content.begin(), content.size() - 1); }
  CAPNP_API inline operator Reader() const { return asReader(); }

  CAPNP_API inline operator kj::ArrayPtr<char>();
  CAPNP_API inline kj::ArrayPtr<char> asArray();
  CAPNP_API inline operator kj::ArrayPtr<const char>() const;
  CAPNP_API inline kj::ArrayPtr<const char> asArray() const;
  CAPNP_API inline kj::ArrayPtr<byte> asBytes() { return asArray().asBytes(); }
  CAPNP_API inline kj::ArrayPtr<const byte> asBytes() const { return asArray().asBytes(); }
  // Result does not include NUL terminator.

  CAPNP_API inline operator kj::StringPtr() const;
  CAPNP_API inline kj::StringPtr asString() const;

  CAPNP_API inline const char* cStr() const { return content.begin(); }
  // Returns NUL-terminated string.

  CAPNP_API inline size_t size() const { return content.size() - 1; }
  // Result does not include NUL terminator.

  CAPNP_API inline char operator[](size_t index) const { return content[index]; }
  CAPNP_API inline char& operator[](size_t index) { return content[index]; }

  CAPNP_API inline char* begin() { return content.begin(); }
  CAPNP_API inline char* end() { return content.end() - 1; }
  CAPNP_API inline const char* begin() const { return content.begin(); }
  CAPNP_API inline const char* end() const { return content.end() - 1; }

  CAPNP_API inline bool operator==(decltype(nullptr)) const { return content.size() <= 1; }
  CAPNP_API inline bool operator!=(decltype(nullptr)) const { return content.size() > 1; }

  CAPNP_API inline bool operator==(Builder other) const { return asString() == other.asString(); }
  CAPNP_API inline bool operator!=(Builder other) const { return asString() != other.asString(); }
  CAPNP_API inline bool operator< (Builder other) const { return asString() <  other.asString(); }
  CAPNP_API inline bool operator> (Builder other) const { return asString() >  other.asString(); }
  CAPNP_API inline bool operator<=(Builder other) const { return asString() <= other.asString(); }
  CAPNP_API inline bool operator>=(Builder other) const { return asString() >= other.asString(); }

  CAPNP_API inline kj::StringPtr slice(size_t start) const;
  CAPNP_API inline kj::ArrayPtr<const char> slice(size_t start, size_t end) const;
  CAPNP_API inline Builder slice(size_t start);
  CAPNP_API inline kj::ArrayPtr<char> slice(size_t start, size_t end);
  // A string slice is only NUL-terminated if it is a suffix, so slice() has a one-parameter
  // version that assumes end = size().

private:
  inline explicit Builder(kj::ArrayPtr<char> content): content(content) {}

  kj::ArrayPtr<char> content;

  static char nulstr[1];
};

CAPNP_API inline kj::StringPtr KJ_STRINGIFY(Text::Builder builder) {
  return builder.asString();
}

CAPNP_API inline bool operator==(const char* a, const Text::Builder& b) {
  return b.asString() == a;
}
CAPNP_API inline bool operator!=(const char* a, const Text::Builder& b) {
  return b.asString() != a;
}

inline Text::Builder::operator kj::StringPtr() const {
  return kj::StringPtr(content.begin(), content.size() - 1);
}

inline kj::StringPtr Text::Builder::asString() const {
  return kj::StringPtr(content.begin(), content.size() - 1);
}

inline Text::Builder::operator kj::ArrayPtr<char>() {
  return content.slice(0, content.size() - 1);
}

inline kj::ArrayPtr<char> Text::Builder::asArray() {
  return content.slice(0, content.size() - 1);
}

inline Text::Builder::operator kj::ArrayPtr<const char>() const {
  return content.slice(0, content.size() - 1);
}

inline kj::ArrayPtr<const char> Text::Builder::asArray() const {
  return content.slice(0, content.size() - 1);
}

inline kj::StringPtr Text::Builder::slice(size_t start) const {
  return asReader().slice(start);
}
inline kj::ArrayPtr<const char> Text::Builder::slice(size_t start, size_t end) const {
  return content.slice(start, end);
}

inline Text::Builder Text::Builder::slice(size_t start) {
  return Text::Builder(content.slice(start, content.size()));
}
inline kj::ArrayPtr<char> Text::Builder::slice(size_t start, size_t end) {
  return content.slice(start, end);
}

}  // namespace capnp

CAPNP_END_HEADER
