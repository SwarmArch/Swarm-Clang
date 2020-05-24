//===- StmtSwarm.h - Classes for Swarm statements ---------------*- C++ -*-===//
//
//                       The SCC Parallelizing Compiler
//
//          Copyright (c) 2020 Massachusetts Institute of Technology
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file defines Swarm AST classes for executable statements and
/// clauses.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_STMTSWARM_H
#define LLVM_CLANG_AST_STMTSWARM_H

#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"

namespace clang {

/// SwarmSpawnStmt - This represents a _Swarm_spawn.
///
class SwarmSpawnStmt : public Stmt {
  SourceLocation SpawnLoc;
  enum { INIT, VAR, TIMESTAMP, SPAWNED, END_EXPR };
  Stmt* SubExprs[END_EXPR];
  enum SwarmSpawnKind { SAME_DOMAIN = 0, SUBDOMAIN, SUPERDOMAIN };
  SwarmSpawnKind Kind = SAME_DOMAIN;

public:
  explicit SwarmSpawnStmt(SourceLocation SL,
                          bool isSub=false, bool isSuper=false)
      : SwarmSpawnStmt(SL, nullptr, isSub, isSuper) {}

  SwarmSpawnStmt(SourceLocation SL, Stmt *S,
                 bool isSub=false, bool isSuper=false)
      : Stmt(SwarmSpawnStmtClass), SpawnLoc(SL) {
    SubExprs[INIT] = nullptr;
    SubExprs[VAR] = nullptr;
    SubExprs[TIMESTAMP] = nullptr;
    SubExprs[SPAWNED] = S;
    setSubdomain(isSub);
    setSuperdomain(isSuper);
  }

  SwarmSpawnStmt(const ASTContext &C, SourceLocation SL,
                 Stmt *Init, VarDecl *Var,
                 Expr *Timestamp,
                 Stmt *SpawnedStmt,
                 bool isSub=false, bool isSuper=false);

  // \brief Build an empty _Swarm_spawn statement.
  explicit SwarmSpawnStmt(EmptyShell Empty)
      : Stmt(SwarmSpawnStmtClass, Empty) { }

  /// \brief Retrieve the variable declared in this "switch" statement, if any.
  ///
  /// In the following example, "x" is the condition variable.
  /// \code
  /// switch (int x = foo()) {
  ///   case 0: break;
  ///   // ...
  /// }
  /// \endcode
  VarDecl *getConditionVariable() const;
  void setConditionVariable(const ASTContext &C, VarDecl *V);

  /// If this SwitchStmt has a condition variable, return the faux DeclStmt
  /// associated with the creation of that condition variable.
  const DeclStmt *getConditionVariableDeclStmt() const {
    return reinterpret_cast<DeclStmt*>(SubExprs[VAR]);
  }

  Stmt *getInit() { return SubExprs[INIT]; }
  const Stmt *getInit() const { return SubExprs[INIT]; }
  void setInit(Stmt *S) { SubExprs[INIT] = S; }
  Expr* getTimestamp() { return reinterpret_cast<Expr*>(SubExprs[TIMESTAMP]); }
  const Expr* getTimestamp() const { return reinterpret_cast<Expr*>(SubExprs[TIMESTAMP]); }
  void setTimestamp(Expr* T) { SubExprs[TIMESTAMP] = reinterpret_cast<Stmt*>(T); }
  Stmt* getSpawnedStmt() { return SubExprs[SPAWNED]; }
  const Stmt* getSpawnedStmt() const { return SubExprs[SPAWNED]; }
  void setSpawnedStmt(Stmt *S) { SubExprs[SPAWNED] = S; }

  bool isSubdomain() const { return Kind == SUBDOMAIN; }
  void setSubdomain(bool sub) {
    if (sub) {
      assert(Kind != SUPERDOMAIN);
      Kind = SUBDOMAIN;
    } else {
      if (Kind == SUBDOMAIN) Kind = SAME_DOMAIN;
    }
  }
  bool isSuperdomain() const { return Kind == SUPERDOMAIN; }
  void setSuperdomain(bool super) {
    if (super) {
      assert(Kind != SUBDOMAIN);
      Kind = SUPERDOMAIN;
    } else {
      if (Kind == SUPERDOMAIN) Kind = SAME_DOMAIN;
    }
  }

  SourceLocation getSpawnLoc() const { return SpawnLoc; }
  void setSpawnLoc(SourceLocation L) { SpawnLoc = L; }

  SourceLocation getLocStart() const LLVM_READONLY { return SpawnLoc; }
  SourceLocation getLocEnd() const LLVM_READONLY {
    return getSpawnedStmt()->getLocEnd();
  }

  static bool classof(const Stmt *T) {
    return T->getStmtClass() == SwarmSpawnStmtClass;
  }

  // Iterators
  child_range children() {
    return child_range(&SubExprs[0], &SubExprs[0]+END_EXPR);
  }
};

//TODO(victory): class SwarmForStmt

}  // end namespace clang

#endif
