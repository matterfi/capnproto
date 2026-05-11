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

#include "export-kj.h"
#include <stddef.h>
#include "common.h"
#include "array.h"
#include "exception.h"
#include <stdint.h>

KJ_BEGIN_HEADER

namespace kj {

// =======================================================================================
// Abstract interfaces

class KJ_CLASS InputStream {
public:
  virtual KJ_API ~InputStream() noexcept(false);

  size_t KJ_API read(void* buffer, size_t minBytes, size_t maxBytes);
  // Reads at least minBytes and at most maxBytes, copying them into the given buffer.  Returns
  // the size read.  Throws an exception on errors.  Implemented in terms of tryRead().
  //
  // maxBytes is the number of bytes the caller really wants, but minBytes is the minimum amount
  // needed by the caller before it can start doing useful processing.  If the stream returns less
  // than maxBytes, the caller will usually call read() again later to get the rest.  Returning
  // less than maxBytes is useful when it makes sense for the caller to parallelize processing
  // with I/O.
  //
  // Never blocks if minBytes is zero.  If minBytes is zero and maxBytes is non-zero, this may
  // attempt a non-blocking read or may just return zero.  To force a read, use a non-zero minBytes.
  // To detect EOF without throwing an exception, use tryRead().
  //
  // If the InputStream can't produce minBytes, it MUST throw an exception, as the caller is not
  // expected to understand how to deal with partial reads.

  virtual size_t KJ_API tryRead(void* buffer, size_t minBytes, size_t maxBytes) = 0;
  // Like read(), but may return fewer than minBytes on EOF.

  inline void KJ_API read(void* buffer, size_t bytes) { read(buffer, bytes, bytes); }
  // Convenience method for reading an exact number of bytes.

  virtual void KJ_API skip(size_t bytes);
  // Skips past the given number of bytes, discarding them.  The default implementation read()s
  // into a scratch buffer.

  String KJ_API readAllText(uint64_t limit = kj::maxValue);
  Array<byte> KJ_API readAllBytes(uint64_t limit = kj::maxValue);
  // Read until EOF and return as one big byte array or string. Throw an exception if EOF is not
  // seen before reading `limit` bytes.
  //
  // To prevent runaway memory allocation, consider using a more conservative value for `limit` than
  // the default, particularly on untrusted data streams which may never see EOF.
};

class KJ_CLASS OutputStream {
public:
  virtual KJ_API ~OutputStream() noexcept(false);

  virtual void KJ_API write(const void* buffer, size_t size) = 0;
  // Always writes the full size.  Throws exception on error.

  virtual void KJ_API write(ArrayPtr<const ArrayPtr<const byte>> pieces);
  // Equivalent to write()ing each byte array in sequence, which is what the default implementation
  // does.  Override if you can do something better, e.g. use writev() to do the write in a single
  // syscall.
};

class KJ_CLASS BufferedInputStream: public InputStream {
  // An input stream which buffers some bytes in memory to reduce system call overhead.
  // - OR -
  // An input stream that actually reads from some in-memory data structure and wants to give its
  // caller a direct pointer to that memory to potentially avoid a copy.

public:
  virtual KJ_API ~BufferedInputStream() noexcept(false);

  ArrayPtr<const byte> KJ_API getReadBuffer();
  // Get a direct pointer into the read buffer, which contains the next bytes in the input.  If the
  // caller consumes any bytes, it should then call skip() to indicate this.  This always returns a
  // non-empty buffer or throws an exception.  Implemented in terms of tryGetReadBuffer().

  virtual ArrayPtr<const byte> KJ_API tryGetReadBuffer() = 0;
  // Like getReadBuffer() but may return an empty buffer on EOF.
};

class KJ_CLASS BufferedOutputStream: public OutputStream {
  // An output stream which buffers some bytes in memory to reduce system call overhead.
  // - OR -
  // An output stream that actually writes into some in-memory data structure and wants to give its
  // caller a direct pointer to that memory to potentially avoid a copy.

public:
  virtual KJ_API ~BufferedOutputStream() noexcept(false);

  virtual ArrayPtr<byte> KJ_API getWriteBuffer() = 0;
  // Get a direct pointer into the write buffer.  The caller may choose to fill in some prefix of
  // this buffer and then pass it to write(), in which case write() may avoid a copy.  It is
  // incorrect to pass to write any slice of this buffer which is not a prefix.
};

// =======================================================================================
// Buffered streams implemented as wrappers around regular streams

class KJ_CLASS BufferedInputStreamWrapper: public BufferedInputStream {
  // Implements BufferedInputStream in terms of an InputStream.
  //
  // Note that the underlying stream's position is unpredictable once the wrapper is destroyed,
  // unless the entire stream was consumed.  To read a predictable number of bytes in a buffered
  // way without going over, you'd need this wrapper to wrap some other wrapper which itself
  // implements an artificial EOF at the desired point.  Such a stream should be trivial to write
  // but is not provided by the library at this time.

public:
  explicit KJ_API BufferedInputStreamWrapper(InputStream& inner, ArrayPtr<byte> buffer = nullptr);
  // Creates a buffered stream wrapping the given non-buffered stream.  No guarantee is made about
  // the position of the inner stream after a buffered wrapper has been created unless the entire
  // input is read.
  //
  // If the second parameter is non-null, the stream uses the given buffer instead of allocating
  // its own.  This may improve performance if the buffer can be reused.

  KJ_DISALLOW_COPY_AND_MOVE(BufferedInputStreamWrapper);
  KJ_API ~BufferedInputStreamWrapper() noexcept(false);

  // implements BufferedInputStream ----------------------------------
  ArrayPtr<const byte> KJ_API tryGetReadBuffer() override;
  size_t KJ_API tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;
  void KJ_API skip(size_t bytes) override;

private:
  InputStream& inner;
  Array<byte> ownedBuffer;
  ArrayPtr<byte> buffer;
  ArrayPtr<byte> bufferAvailable;
};

class KJ_CLASS BufferedOutputStreamWrapper: public BufferedOutputStream {
  // Implements BufferedOutputStream in terms of an OutputStream.  Note that writes to the
  // underlying stream may be delayed until flush() is called or the wrapper is destroyed.

public:
  explicit KJ_API BufferedOutputStreamWrapper(OutputStream& inner,
                                              ArrayPtr<byte> buffer = nullptr);
  // Creates a buffered stream wrapping the given non-buffered stream.
  //
  // If the second parameter is non-null, the stream uses the given buffer instead of allocating
  // its own.  This may improve performance if the buffer can be reused.

  KJ_DISALLOW_COPY_AND_MOVE(BufferedOutputStreamWrapper);
  KJ_API ~BufferedOutputStreamWrapper() noexcept(false);

  void KJ_API flush();
  // Force the wrapper to write any remaining bytes in its buffer to the inner stream.  Note that
  // this only flushes this object's buffer; this object has no idea how to flush any other buffers
  // that may be present in the underlying stream.

  // implements BufferedOutputStream ---------------------------------
  ArrayPtr<byte> KJ_API getWriteBuffer() override;
  void KJ_API write(const void* buffer, size_t size) override;

private:
  OutputStream& inner;
  Array<byte> ownedBuffer;
  ArrayPtr<byte> buffer;
  byte* bufferPos;
  UnwindDetector unwindDetector;
};

// =======================================================================================
// Array I/O

class ArrayInputStream: public BufferedInputStream {
public:
  explicit KJ_API ArrayInputStream(ArrayPtr<const byte> array);
  KJ_DISALLOW_COPY_AND_MOVE(ArrayInputStream);
  KJ_API ~ArrayInputStream() noexcept(false);

  // implements BufferedInputStream ----------------------------------
  ArrayPtr<const byte> KJ_API tryGetReadBuffer() override;
  size_t KJ_API tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;
  void KJ_API skip(size_t bytes) override;

private:
  ArrayPtr<const byte> array;
};

class KJ_CLASS ArrayOutputStream: public BufferedOutputStream {
public:
  explicit KJ_API ArrayOutputStream(ArrayPtr<byte> array);
  KJ_DISALLOW_COPY_AND_MOVE(ArrayOutputStream);
  KJ_API ~ArrayOutputStream() noexcept(false);

  ArrayPtr<byte> KJ_API getArray() {
    // Get the portion of the array which has been filled in.
    return arrayPtr(array.begin(), fillPos);
  }

  // implements BufferedInputStream ----------------------------------
  ArrayPtr<byte> KJ_API getWriteBuffer() override;
  void KJ_API write(const void* buffer, size_t size) override;

private:
  ArrayPtr<byte> array;
  byte* fillPos;
};

class KJ_CLASS VectorOutputStream: public BufferedOutputStream {
public:
  explicit KJ_API VectorOutputStream(size_t initialCapacity = 4096);
  KJ_DISALLOW_COPY_AND_MOVE(VectorOutputStream);
  KJ_API ~VectorOutputStream() noexcept(false);

  ArrayPtr<byte> KJ_API getArray() {
    // Get the portion of the array which has been filled in.
    return arrayPtr(vector.begin(), fillPos);
  }

  void KJ_API clear() { fillPos = vector.begin(); }

  // implements BufferedInputStream ----------------------------------
  ArrayPtr<byte> KJ_API getWriteBuffer() override;
  void KJ_API write(const void* buffer, size_t size) override;

private:
  Array<byte> vector;
  byte* fillPos;

  void grow(size_t minSize);
};

// =======================================================================================
// File descriptor I/O

class KJ_CLASS AutoCloseFd {
  // A wrapper around a file descriptor which automatically closes the descriptor when destroyed.
  // The wrapper supports move construction for transferring ownership of the descriptor.  If
  // close() returns an error, the destructor throws an exception, UNLESS the destructor is being
  // called during unwind from another exception, in which case the close error is ignored.
  //
  // If your code is not exception-safe, you should not use AutoCloseFd.  In this case you will
  // have to call close() yourself and handle errors appropriately.

public:
  inline KJ_API AutoCloseFd(): fd(-1) {}
  inline KJ_API AutoCloseFd(decltype(nullptr)): fd(-1) {}
  inline explicit KJ_API AutoCloseFd(int fd): fd(fd) {}
  inline KJ_API AutoCloseFd(AutoCloseFd&& other) noexcept: fd(other.fd) { other.fd = -1; }
  KJ_DISALLOW_COPY(AutoCloseFd);
  KJ_API ~AutoCloseFd() noexcept(false);

  inline AutoCloseFd& KJ_API operator=(AutoCloseFd&& other) {
    AutoCloseFd old(kj::mv(*this));
    fd = other.fd;
    other.fd = -1;
    return *this;
  }

  inline AutoCloseFd& KJ_API operator=(decltype(nullptr)) {
    AutoCloseFd old(kj::mv(*this));
    return *this;
  }

  inline KJ_API operator int() const { return fd; }
  inline int KJ_API get() const { return fd; }

  operator bool() const = delete;
  // Deleting this operator prevents accidental use in boolean contexts, which
  // the int conversion operator above would otherwise allow.

  inline bool KJ_API operator==(decltype(nullptr)) { return fd < 0; }
  inline bool KJ_API operator!=(decltype(nullptr)) { return fd >= 0; }

  inline int KJ_API release() {
    // Release ownership of an FD. Not recommended.
    int result = fd;
    fd = -1;
    return result;
  }

private:
  int fd;
};

inline auto KJ_API KJ_STRINGIFY(const AutoCloseFd& fd)
    -> decltype(kj::toCharSequence(implicitCast<int>(fd))) {
  return kj::toCharSequence(implicitCast<int>(fd));
}

class KJ_CLASS FdInputStream: public InputStream {
  // An InputStream wrapping a file descriptor.

public:
  explicit KJ_API FdInputStream(int fd): fd(fd) {}
  explicit KJ_API FdInputStream(AutoCloseFd fd): fd(fd), autoclose(mv(fd)) {}
  KJ_DISALLOW_COPY_AND_MOVE(FdInputStream);
  KJ_API ~FdInputStream() noexcept(false);

  size_t KJ_API tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;

  inline int KJ_API getFd() const { return fd; }

private:
  int fd;
  AutoCloseFd autoclose;
};

class KJ_CLASS FdOutputStream: public OutputStream {
  // An OutputStream wrapping a file descriptor.

public:
  explicit KJ_API FdOutputStream(int fd): fd(fd) {}
  explicit KJ_API FdOutputStream(AutoCloseFd fd): fd(fd), autoclose(mv(fd)) {}
  KJ_DISALLOW_COPY_AND_MOVE(FdOutputStream);
  KJ_API ~FdOutputStream() noexcept(false);

  void KJ_API write(const void* buffer, size_t size) override;
  void KJ_API write(ArrayPtr<const ArrayPtr<const byte>> pieces) override;

  inline int KJ_API getFd() const { return fd; }

private:
  int fd;
  AutoCloseFd autoclose;
};

// =======================================================================================
// Win32 Handle I/O

#ifdef _WIN32

class KJ_CLASS AutoCloseHandle {
  // A wrapper around a Win32 HANDLE which automatically closes the handle when destroyed.
  // The wrapper supports move construction for transferring ownership of the handle.  If
  // CloseHandle() returns an error, the destructor throws an exception, UNLESS the destructor is
  // being called during unwind from another exception, in which case the close error is ignored.
  //
  // If your code is not exception-safe, you should not use AutoCloseHandle.  In this case you will
  // have to call close() yourself and handle errors appropriately.

public:
  inline KJ_API AutoCloseHandle(): handle((void*)-1) {}
  inline KJ_API AutoCloseHandle(decltype(nullptr)): handle((void*)-1) {}
  inline explicit KJ_API AutoCloseHandle(void* handle): handle(handle) {}
  inline KJ_API AutoCloseHandle(AutoCloseHandle&& other) noexcept: handle(other.handle) {
    other.handle = (void*)-1;
  }
  KJ_DISALLOW_COPY(AutoCloseHandle);
  KJ_API ~AutoCloseHandle() noexcept(false);

  inline AutoCloseHandle& KJ_API operator=(AutoCloseHandle&& other) {
    AutoCloseHandle old(kj::mv(*this));
    handle = other.handle;
    other.handle = (void*)-1;
    return *this;
  }

  inline AutoCloseHandle& KJ_API operator=(decltype(nullptr)) {
    AutoCloseHandle old(kj::mv(*this));
    return *this;
  }

  inline KJ_API operator void*() const { return handle; }
  inline void* KJ_API get() const { return handle; }

  operator bool() const = delete;
  // Deleting this operator prevents accidental use in boolean contexts, which
  // the void* conversion operator above would otherwise allow.

  inline bool KJ_API operator==(decltype(nullptr)) { return handle != (void*)-1; }
  inline bool KJ_API operator!=(decltype(nullptr)) { return handle == (void*)-1; }

  inline void* KJ_API release() {
    // Release ownership of an FD. Not recommended.
    void* result = handle;
    handle = (void*)-1;
    return result;
  }

private:
  void* handle;  // -1 (aka INVALID_HANDLE_VALUE) if not valid.
};

class KJ_CLASS HandleInputStream: public InputStream {
  // An InputStream wrapping a Win32 HANDLE.

public:
  explicit KJ_API HandleInputStream(void* handle): handle(handle) {}
  explicit KJ_API HandleInputStream(AutoCloseHandle handle): handle(handle), autoclose(mv(handle)) {}
  KJ_DISALLOW_COPY_AND_MOVE(HandleInputStream);
  KJ_API ~HandleInputStream() noexcept(false);

  size_t KJ_API tryRead(void* buffer, size_t minBytes, size_t maxBytes) override;

private:
  void* handle;
  AutoCloseHandle autoclose;
};

class KJ_CLASS HandleOutputStream: public OutputStream {
  // An OutputStream wrapping a Win32 HANDLE.

public:
  explicit KJ_API HandleOutputStream(void* handle): handle(handle) {}
  explicit KJ_API HandleOutputStream(AutoCloseHandle handle): handle(handle), autoclose(mv(handle)) {}
  KJ_DISALLOW_COPY_AND_MOVE(HandleOutputStream);
  KJ_API ~HandleOutputStream() noexcept(false);

  void KJ_API write(const void* buffer, size_t size) override;

private:
  void* handle;
  AutoCloseHandle autoclose;
};

#endif  // _WIN32

}  // namespace kj

KJ_END_HEADER
