// Copyright (c) 2017 Cloudflare, Inc. and contributors
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

#include <kj/export-kj-gzip.h>
#include <kj/io.h>
#include <kj/async-io.h>
#include <zlib.h>

KJ_BEGIN_HEADER

namespace kj {

namespace _ {  // private

constexpr size_t KJ_GZ_BUF_SIZE = 4096;

class KJ_GZIP_CLASS GzipOutputContext final {
public:
  KJ_GZIP_API GzipOutputContext(kj::Maybe<int> compressionLevel);
  KJ_GZIP_API ~GzipOutputContext() noexcept(false);
  KJ_DISALLOW_COPY_AND_MOVE(GzipOutputContext);

  KJ_GZIP_API void setInput(const void* in, size_t size);
  KJ_GZIP_API kj::Tuple<bool, kj::ArrayPtr<const byte>> pumpOnce(int flush);

private:
  bool compressing;
  z_stream ctx = {};
  byte buffer[_::KJ_GZ_BUF_SIZE];

  [[noreturn]] void fail(int result);
};

}  // namespace _ (private)

class KJ_GZIP_CLASS GzipInputStream final: public InputStream {
public:
  KJ_GZIP_API GzipInputStream(InputStream& inner);
  KJ_GZIP_API ~GzipInputStream() noexcept(false);
  KJ_DISALLOW_COPY_AND_MOVE(GzipInputStream);

  KJ_GZIP_API size_t tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;

private:
  InputStream& inner;
  z_stream ctx = {};
  bool atValidEndpoint = false;

  byte buffer[_::KJ_GZ_BUF_SIZE];

  size_t readImpl(byte* buffer, size_t minBytes, size_t maxBytes, size_t alreadyRead);
};

class KJ_GZIP_CLASS GzipOutputStream final: public OutputStream {
public:
  enum { DECOMPRESS };

  KJ_GZIP_API GzipOutputStream(OutputStream& inner, int compressionLevel = Z_DEFAULT_COMPRESSION);
  KJ_GZIP_API GzipOutputStream(OutputStream& inner, decltype(DECOMPRESS));
  KJ_GZIP_API ~GzipOutputStream() noexcept(false);
  KJ_DISALLOW_COPY_AND_MOVE(GzipOutputStream);

  KJ_GZIP_API void write(const void* buffer, size_t size) override;
  using OutputStream::write;

  KJ_GZIP_API inline void flush() {
    pump(Z_SYNC_FLUSH);
  }

private:
  OutputStream& inner;
  _::GzipOutputContext ctx;

  void pump(int flush);
};

class KJ_GZIP_CLASS GzipAsyncInputStream final: public AsyncInputStream {
public:
  KJ_GZIP_API GzipAsyncInputStream(AsyncInputStream& inner);
  KJ_GZIP_API ~GzipAsyncInputStream() noexcept(false);
  KJ_DISALLOW_COPY_AND_MOVE(GzipAsyncInputStream);

  KJ_GZIP_API Promise<size_t> tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;

private:
  AsyncInputStream& inner;
  z_stream ctx = {};
  bool atValidEndpoint = false;

  byte buffer[_::KJ_GZ_BUF_SIZE];

  Promise<size_t> readImpl(byte* buffer, size_t minBytes, size_t maxBytes, size_t alreadyRead);
};

class KJ_GZIP_CLASS GzipAsyncOutputStream final: public AsyncOutputStream {
public:
  enum { DECOMPRESS };

  KJ_GZIP_API GzipAsyncOutputStream(AsyncOutputStream& inner,
                                    int compressionLevel = Z_DEFAULT_COMPRESSION);
  KJ_GZIP_API GzipAsyncOutputStream(AsyncOutputStream& inner, decltype(DECOMPRESS));
  KJ_DISALLOW_COPY_AND_MOVE(GzipAsyncOutputStream);

  KJ_GZIP_API Promise<void> write(const void* buffer, size_t size) override;
  KJ_GZIP_API Promise<void> write(ArrayPtr<const ArrayPtr<const byte>> pieces) override;

  KJ_GZIP_API Promise<void> whenWriteDisconnected() override {
      return inner.whenWriteDisconnected();
  }

  KJ_GZIP_API inline Promise<void> flush() {
    return pump(Z_SYNC_FLUSH);
  }
  // Call if you need to flush a stream at an arbitrary data point.

  KJ_GZIP_API Promise<void> end() {
    return pump(Z_FINISH);
  }
  // Must call to flush and finish the stream, since some data may be buffered.
  //
  // TODO(cleanup): This should be a virtual method on AsyncOutputStream.

private:
  AsyncOutputStream& inner;
  _::GzipOutputContext ctx;

  kj::Promise<void> pump(int flush);
};

}  // namespace kj

KJ_END_HEADER
