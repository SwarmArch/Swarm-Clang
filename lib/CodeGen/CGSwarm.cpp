//===--- CGSwarm.cpp - Emit LLVM Code for Swarm expressions ---------------===//
//
//                       The SCC Parallelizing Compiler
//
//          Copyright (c) 2020 Massachusetts Institute of Technology
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with code generation of Swarm statements and
// expressions.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGCleanup.h"
#include "clang/AST/StmtSwarm.h"

using namespace clang;
using namespace CodeGen;

void CodeGenFunction::EmitSwarmSpawnStmt(const SwarmSpawnStmt &S) {
  RunCleanupsScope ConditionScope(*this);

  if (S.getInit())
    EmitStmt(S.getInit());

  if (S.getConditionVariable())
    EmitAutoVarDecl(*S.getConditionVariable());
  llvm::Value *TimestampV = nullptr;
  if (S.getTimestamp())
    TimestampV = EmitScalarExpr(S.getTimestamp());

  // TODO(victory): Handle spawning of calls in a special manner, to evaluate
  // arguments before spawn?

  // Set up to perform a detach.
  PushDetachScope();
  CurDetachScope->StartDetach(true, TimestampV, S.isSubdomain(),
                              S.isSuperdomain());

  // Emit the spawned statement
  EmitStmt(S.getSpawnedStmt());

  // Finish the detach.
  PopDetachScope(true);

  ConditionScope.ForceCleanup();
}

//TODO(victory):
//void CodeGenFunction::EmitSwarmForStmt(const CilkForStmt &S,
//                                      ArrayRef<const Attr *> ForAttrs) {

