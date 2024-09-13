//===- CXNested.h - Routines for manipulating CXNested ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines routines for manipulating CXNested.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_LIBCLANG_CXNESTED_H
#define LLVM_CLANG_TOOLS_LIBCLANG_CXNESTED_H

#include "clang-c/Index.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

namespace clang {
namespace cxnested {

CXNested
MakeCXNestedInvalid(CXTranslationUnit TU = nullptr);

CXNested MakeCXNested_DependentNameType(const DependentNameType *Ptr,
                                         CXTranslationUnit TU);

CXNested MakeCXNested_DependentTemplateSpecializationType(
    const DependentTemplateSpecializationType *Ptr, CXTranslationUnit TU);

CXNested
MakeCXNested_DependentTemplateName(const DependentTemplateName *Ptr,
                                 CXTranslationUnit TU);

template <typename T> const T *GetNestedPtr(CXNested CN);

CXTranslationUnit GetTU(CXNested CN);

}
} // namespace clang
#endif
