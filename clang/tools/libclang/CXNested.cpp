//===- CXNested.cpp - Implements 'CXNested' aspect of libclang ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===--------------------------------------------------------------------===//
//
// This file implements the 'CXNested' API hooks in the Clang-C library.
//
//===--------------------------------------------------------------------===//

#include "CXType.h"
#include "CIndexer.h"
#include "CXCursor.h"
#include "CXString.h"
#include "CXNested.h"
#include "CXTranslationUnit.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"

using namespace clang;

namespace {

template <typename T> struct NestedKindFor {};

template <> struct NestedKindFor<DependentNameType> {
  static const CXNestedKind value = CXNested_DependentNameType;
};

template <> struct NestedKindFor<DependentTemplateSpecializationType> {
  static const CXNestedKind value =
      CXNested_DependentTemplateSpecializationType;
};

template <> struct NestedKindFor<DependentTemplateName> {
  static const CXNestedKind value = CXNested_DependentTemplateName;
};

inline CXNested MakeCXNested(CXNestedKind Kind, const void *Ptr,
                             CXTranslationUnit TU) {
  return {Kind, const_cast<void *>(Ptr), TU};
}

template <typename T>
inline CXNested MakeCXNested_(const T *Ptr, CXTranslationUnit TU) {
    return MakeCXNested(NestedKindFor<T>::value, Ptr, TU);
}

CXNestedKind GetNestedNameSpecifierKind(const clang::NestedNameSpecifier *NNS) {
  if (!NNS)
    return CXNested_Invalid;

  switch (NNS->getKind()) {
  case NestedNameSpecifier::Identifier:
    return CXNested_Identifier;
  case NestedNameSpecifier::Namespace:
    return CXNested_Namespace;
  case NestedNameSpecifier::NamespaceAlias:
    return CXNested_NamespaceAlias;
  case NestedNameSpecifier::TypeSpec:
    return CXNested_TypeSpec;
  case NestedNameSpecifier::TypeSpecWithTemplate:
    return CXNested_TypeSpecWithTemplate;
  case NestedNameSpecifier::Global:
    return CXNested_Global;
  case NestedNameSpecifier::Super:
    return CXNested_Super;
  default:
    return CXNested_Invalid;
  }
}

inline constexpr bool IsNestedNameSpecifierKind(CXNestedKind Kind) {
  switch (Kind) {
  case CXNested_Identifier:
  case CXNested_Namespace:
  case CXNested_NamespaceAlias:
  case CXNested_TypeSpec:
  case CXNested_TypeSpecWithTemplate:
  case CXNested_Global:
  case CXNested_Super:
    return true;

  default:
    return false;
  }
}

}

namespace clang::cxnested {

CXNested
MakeCXNestedInvalid(CXTranslationUnit TU) {
  return MakeCXNested(CXNested_Invalid, nullptr, TU);
}

CXNested MakeCXNested_NestedNameSpecifier(const NestedNameSpecifier *Ptr,
                                        CXTranslationUnit TU) {
  return MakeCXNested(GetNestedNameSpecifierKind(Ptr), Ptr, TU);
}

CXNested MakeCXNested_DependentNameType(const DependentNameType *Ptr,
                                                CXTranslationUnit TU) {
  return MakeCXNested_(Ptr, TU);
}

CXNested MakeCXNested_DependentTemplateSpecializationType(
    const DependentTemplateSpecializationType *Ptr, CXTranslationUnit TU) {
  return MakeCXNested_(Ptr, TU);
}

CXNested
MakeCXNested_DependentTemplateName(const DependentTemplateName *Ptr,
                                 CXTranslationUnit TU) {
  return MakeCXNested_(Ptr, TU);
}

template <typename T> inline const T *GetNestedPtr(CXNested CN) {
  assert(CN.kind == NestedKindFor<T>::value);
  return static_cast<T *>(CN.data[0]);
}

template const DependentNameType *GetNestedPtr<DependentNameType>(CXNested CN);

template const DependentTemplateSpecializationType *
GetNestedPtr<DependentTemplateSpecializationType>(CXNested CN);

template const DependentTemplateName *
GetNestedPtr<DependentTemplateName>(CXNested CN);

template <>
inline const NestedNameSpecifier *
GetNestedPtr<NestedNameSpecifier>(CXNested CN) {
  assert(IsNestedNameSpecifierKind(CN.kind));
  return static_cast<NestedNameSpecifier *>(CN.data[0]);
}

inline CXTranslationUnit GetTU(CXNested CN) {
  return static_cast<CXTranslationUnit>(CN.data[1]);
}

CXIdentifier MakeCXIdentifier(const clang::IdentifierInfo *IF) {
  return reinterpret_cast<CXIdentifier>(IF);
}

const clang::IdentifierInfo *GetIdentifier(CXIdentifier Identifier) {
  return reinterpret_cast<const clang::IdentifierInfo *>(Identifier);
}

}


CXNestedKind clang_Nested_getKind(CXNested CN) { return CN.kind; }

CXNested clang_Nested_getPrefix(CXNested CN) {
  using namespace cxnested;

  auto TU = GetTU(CN);

  switch (CN.kind) {
  case CXNested_Identifier:
  case CXNested_Namespace:
  case CXNested_NamespaceAlias:
  case CXNested_TypeSpec:
  case CXNested_TypeSpecWithTemplate:
  case CXNested_Global:
  case CXNested_Super:
    return MakeCXNested_NestedNameSpecifier(GetNestedPtr<NestedNameSpecifier>(CN)->getPrefix(), TU);

  case CXNested_DependentNameType:
    return MakeCXNested_NestedNameSpecifier(
        GetNestedPtr<DependentNameType>(CN)->getQualifier(),
                        TU);

  case CXNested_DependentTemplateSpecializationType:
    return MakeCXNested_NestedNameSpecifier(
        GetNestedPtr<DependentTemplateSpecializationType>(CN)->getQualifier(),
        TU);

  case CXNested_DependentTemplateName:
    return MakeCXNested_NestedNameSpecifier(
        GetNestedPtr<DependentTemplateName>(CN)->getQualifier(),
                        TU);

  case CXNested_Invalid:
    return MakeCXNestedInvalid(TU);
  }
  llvm_unreachable("Invalid CXNestedKind!");
}

CXType clang_Nested_getType(CXNested CN) {
  using namespace cxnested;

  auto TU = GetTU(CN);

  switch (CN.kind) {
  case CXNested_TypeSpec:
  case CXNested_TypeSpecWithTemplate:
    return cxtype::MakeCXType(
        QualType(GetNestedPtr<NestedNameSpecifier>(CN)->getAsType(), 0), TU);

  case CXNested_DependentNameType:
    return cxtype::MakeCXType(QualType(GetNestedPtr<DependentNameType>(CN), 0),
                              TU);

  case CXNested_DependentTemplateSpecializationType:
    return cxtype::MakeCXType(
        QualType(GetNestedPtr<DependentTemplateSpecializationType>(CN), 0), TU);

  default:
    return cxtype::MakeCXType(QualType(), TU);
  }
}

CXIdentifier clang_Nested_getIdentifier(CXNested CN) {
  using namespace cxnested;

  switch (CN.kind) {
  case CXNested_Identifier:
    return MakeCXIdentifier(GetNestedPtr<NestedNameSpecifier>(CN)->getAsIdentifier());

  case CXNested_DependentNameType:
    return MakeCXIdentifier(
        GetNestedPtr<DependentNameType>(CN)->getIdentifier());

  case CXNested_DependentTemplateSpecializationType:
    return MakeCXIdentifier(
        GetNestedPtr<DependentTemplateSpecializationType>(CN)
            ->getIdentifier());

  case CXNested_DependentTemplateName:
    return MakeCXIdentifier(
        GetNestedPtr<DependentTemplateName>(CN)->getIdentifier());

  default:
    return MakeCXIdentifier(nullptr);
  }
  llvm_unreachable("invalid CXNestedKind");
}

CXString clang_Identifier_getName(CXIdentifier CI) {
  using namespace cxnested;

  auto II = GetIdentifier(CI);
  assert(II);
  return cxstring::createDup(II->getName());
}
