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
  inline CAPNP_API Reader(decltype(nullptr)): ArrayPtr<const byte>(nullptr) {}
  inline CAPNP_API Reader(const byte* value, size_t size): ArrayPtr<const byte>(value, size) {}
  inline CAPNP_API Reader(const kj::Array<const byte>& value): ArrayPtr<const byte>(value) {}
  inline CAPNP_API Reader(const ArrayPtr<const byte>& value): ArrayPtr<const byte>(value) {}
  inline CAPNP_API Reader(const kj::Array<byte>& value): ArrayPtr<const byte>(value) {}
  inline CAPNP_API Reader(const ArrayPtr<byte>& value): ArrayPtr<const byte>(value) {}
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
  inline CAPNP_API Reader(decltype(nullptr)): StringPtr(nullptr) {}
  inline CAPNP_API Reader(const char* value): StringPtr(value) {}
  inline CAPNP_API Reader(const char* value, size_t size): StringPtr(value, size) {}
  inline CAPNP_API Reader(const kj::String& value): StringPtr(value) {}
  inline CAPNP_API Reader(const StringPtr& value): StringPtr(value) {}

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
  inline CAPNP_API Builder(decltype(nullptr)): ArrayPtr<byte>(nullptr) {}
  inline CAPNP_API Builder(byte* value, size_t size): ArrayPtr<byte>(value, size) {}
  inline CAPNP_API Builder(kj::Array<byte>& value): ArrayPtr<byte>(value) {}
  inline CAPNP_API Builder(ArrayPtr<byte> value): ArrayPtr<byte>(value) {}

  inline Data::Reader CAPNP_API asReader() const {
    return Data::Reader(kj::implicitCast<const kj::ArrayPtr<byte>&>(*this));
  }
  inline CAPNP_API operator Reader() const { return asReader(); }
};

class CAPNP_CLASS Text::Builder: public kj::DisallowConstCopy {
  // Basically identical to kj::StringPtr, except that the contents are non-const.

public:
  inline CAPNP_API Builder(): content(nulstr, 1) {}
  inline CAPNP_API Builder(decltype(nullptr)): content(nulstr, 1) {}
  inline CAPNP_API Builder(char* value): content(value, strlen(value) + 1) {}
  inline CAPNP_API Builder(char* value, size_t size): content(value, size + 1) {
    KJ_IREQUIRE(value[size] == '\0', "StringPtr must be NUL-terminated.");
  }

  inline Reader CAPNP_API asReader() const { return Reader(content.begin(), content.size() - 1); }
  inline CAPNP_API operator Reader() const { return asReader(); }

  inline CAPNP_API operator kj::ArrayPtr<char>();
  inline kj::ArrayPtr<char> CAPNP_API asArray();
  inline CAPNP_API operator kj::ArrayPtr<const char>() const;
  inline kj::ArrayPtr<const char> CAPNP_API asArray() const;
  inline kj::ArrayPtr<byte> CAPNP_API asBytes() { return asArray().asBytes(); }
  inline kj::ArrayPtr<const byte> CAPNP_API asBytes() const { return asArray().asBytes(); }
  // Result does not include NUL terminator.

  inline CAPNP_API operator kj::StringPtr() const;
  inline kj::StringPtr CAPNP_API asString() const;

  inline const char* CAPNP_API cStr() const { return content.begin(); }
  // Returns NUL-terminated string.

  inline size_t CAPNP_API size() const { return content.size() - 1; }
  // Result does not include NUL terminator.

  inline char CAPNP_API operator[](size_t index) const { return content[index]; }
  inline char& CAPNP_API operator[](size_t index) { return content[index]; }

  inline char* CAPNP_API begin() { return content.begin(); }
  inline char* CAPNP_API end() { return content.end() - 1; }
  inline const char* CAPNP_API begin() const { return content.begin(); }
  inline const char* CAPNP_API end() const { return content.end() - 1; }

  inline bool CAPNP_API operator==(decltype(nullptr)) const { return content.size() <= 1; }
  inline bool CAPNP_API operator!=(decltype(nullptr)) const { return content.size() > 1; }

  inline bool CAPNP_API operator==(Builder other) const { return asString() == other.asString(); }
  inline bool CAPNP_API operator!=(Builder other) const { return asString() != other.asString(); }
  inline bool CAPNP_API operator< (Builder other) const { return asString() <  other.asString(); }
  inline bool CAPNP_API operator> (Builder other) const { return asString() >  other.asString(); }
  inline bool CAPNP_API operator<=(Builder other) const { return asString() <= other.asString(); }
  inline bool CAPNP_API operator>=(Builder other) const { return asString() >= other.asString(); }

  inline kj::StringPtr CAPNP_API slice(size_t start) const;
  inline kj::ArrayPtr<const char> CAPNP_API slice(size_t start, size_t end) const;
  inline Builder CAPNP_API slice(size_t start);
  inline kj::ArrayPtr<char> CAPNP_API slice(size_t start, size_t end);
  // A string slice is only NUL-terminated if it is a suffix, so slice() has a one-parameter
  // version that assumes end = size().

private:
  inline explicit Builder(kj::ArrayPtr<char> content): content(content) {}

  kj::ArrayPtr<char> content;

  static char nulstr[1];
};

inline kj::StringPtr CAPNP_API KJ_STRINGIFY(Text::Builder builder) {
  return builder.asString();
}

inline bool CAPNP_API operator==(const char* a, const Text::Builder& b) {
  return b.asString() == a;
}
inline bool CAPNP_API operator!=(const char* a, const Text::Builder& b) {
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
