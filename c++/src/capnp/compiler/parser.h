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

#include <capnp/export-capnp-capnpc.h>
#include <capnp/compiler/grammar.capnp.h>
#include <capnp/compiler/lexer.capnp.h>
#include <kj/parse/common.h>
#include <kj/arena.h>
#include "error-reporter.h"

CAPNP_BEGIN_HEADER

namespace capnp {
namespace compiler {

void CAPNP_CAPNPC_API parseFile(List<Statement>::Reader statements, ParsedFile::Builder result,
                                ErrorReporter& errorReporter, bool requiresId);
// Parse a list of statements to build a ParsedFile.
//
// If any errors are reported, then the output is not usable.  However, it may be passed on through
// later stages of compilation in order to detect additional errors.

uint64_t CAPNP_CAPNPC_API generateRandomId();
// Generate a new random unique ID.  This lives here mostly for lack of a better location.

uint64_t CAPNP_CAPNPC_API generateChildId(uint64_t parentId, kj::StringPtr childName);
// Generate the ID for a child node given its parent ID and name.

uint64_t CAPNP_CAPNPC_API generateGroupId(uint64_t parentId, uint16_t groupIndex);
// Generate the ID for a group within a struct.

uint64_t CAPNP_CAPNPC_API generateMethodParamsId(uint64_t parentId, uint16_t methodOrdinal,
                                                 bool isResults);
// Generate the ID for a struct representing method params / results.
//
// TODO(cleanup):  Move generate*Id() somewhere more sensible.

class CAPNP_CAPNPC_CLASS CapnpParser {
  // Advanced parser interface.  This interface exposes the inner parsers so that you can embed
  // them into your own parsers.

public:
  CAPNP_CAPNPC_API CapnpParser(Orphanage orphanage, ErrorReporter& errorReporter);
  // `orphanage` is used to allocate Cap'n Proto message objects in the result.  `inputStart` is
  // a pointer to the beginning of the input, used to compute byte offsets.

  CAPNP_CAPNPC_API ~CapnpParser() noexcept(false);

  KJ_DISALLOW_COPY_AND_MOVE(CapnpParser);

  using ParserInput = kj::parse::IteratorInput<Token::Reader, List<Token>::Reader::Iterator>;
  struct DeclParserResult;
  template <typename Output>
  using Parser = kj::parse::ParserRef<ParserInput, Output>;
  using DeclParser = Parser<DeclParserResult>;

  kj::Maybe<Orphan<Declaration>> CAPNP_CAPNPC_API parseStatement(
      Statement::Reader statement, const DeclParser& parser);
  // Parse a statement using the given parser.  In addition to parsing the token sequence itself,
  // this takes care of parsing the block (if any) and copying over the doc comment (if any).

  struct CAPNP_CAPNPC_CLASS DeclParserResult {
    // DeclParser parses a sequence of tokens representing just the "line" part of the statement --
    // i.e. everything up to the semicolon or opening curly brace.
    //
    // Use `parseStatement()` to avoid having to deal with this struct.

    Orphan<Declaration> CAPNP_CAPNPC_API decl;
    // The decl parsed so far.  The decl's `docComment` and `nestedDecls` are both empty at this
    // point.

    kj::Maybe<DeclParser> CAPNP_CAPNPC_API memberParser;
    // If null, the statement should not have a block.  If non-null, the statement should have a
    // block containing statements parseable by this parser.

    CAPNP_CAPNPC_API DeclParserResult(Orphan<Declaration>&& decl, const DeclParser& memberParser)
        : decl(kj::mv(decl)), memberParser(memberParser) {}
    explicit CAPNP_CAPNPC_API DeclParserResult(Orphan<Declaration>&& decl)
        : decl(kj::mv(decl)), memberParser(nullptr) {}
  };

  struct CAPNP_CAPNPC_CLASS Parsers {
    DeclParser CAPNP_CAPNPC_API genericDecl;
    // Parser that matches any declaration type except those that have ordinals (since they are
    // context-dependent).

    DeclParser CAPNP_CAPNPC_API fileLevelDecl;
    DeclParser CAPNP_CAPNPC_API enumLevelDecl;
    DeclParser CAPNP_CAPNPC_API structLevelDecl;
    DeclParser CAPNP_CAPNPC_API interfaceLevelDecl;
    // Parsers that match genericDecl *and* the ordinal-based declaration types valid in the given
    // contexts.  Note that these may match declarations that are not actually allowed in the given
    // contexts, as long as the grammar is unambiguous.  E.g. nested types are not allowed in
    // enums, but they'll be accepted by enumLevelDecl.  A later stage of compilation should report
    // these as errors.

    Parser<Orphan<Expression>> CAPNP_CAPNPC_API expression;
    Parser<Orphan<Declaration::AnnotationApplication>> CAPNP_CAPNPC_API annotation;
    Parser<Orphan<LocatedInteger>> CAPNP_CAPNPC_API uid;
    Parser<Orphan<LocatedInteger>> CAPNP_CAPNPC_API ordinal;
    Parser<Orphan<Declaration::Param>> CAPNP_CAPNPC_API param;

    DeclParser CAPNP_CAPNPC_API usingDecl;
    DeclParser CAPNP_CAPNPC_API constDecl;
    DeclParser CAPNP_CAPNPC_API enumDecl;
    DeclParser CAPNP_CAPNPC_API enumerantDecl;
    DeclParser CAPNP_CAPNPC_API structDecl;
    DeclParser CAPNP_CAPNPC_API fieldDecl;
    DeclParser CAPNP_CAPNPC_API unionDecl;
    DeclParser CAPNP_CAPNPC_API groupDecl;
    DeclParser CAPNP_CAPNPC_API interfaceDecl;
    DeclParser CAPNP_CAPNPC_API methodDecl;
    DeclParser CAPNP_CAPNPC_API paramDecl;
    DeclParser CAPNP_CAPNPC_API annotationDecl;
    // Parsers for individual declaration types.
  };

  const Parsers& CAPNP_CAPNPC_API getParsers() { return parsers; }

private:
  Orphanage orphanage;
  ErrorReporter& errorReporter;
  kj::Arena arena;
  Parsers parsers;
};

kj::String CAPNP_CAPNPC_API expressionString(Expression::Reader name);
// Stringify the expression as code.

}  // namespace compiler
}  // namespace capnp

CAPNP_END_HEADER
