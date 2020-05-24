//===--- SemaSwarm.cpp - Semantic analysis for Swarm extensions -----------===//
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
//  This file implements semantic analysis for Swarm extensions.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/StmtSwarm.h"
#include "clang/Sema/SemaInternal.h"

using namespace clang;
using namespace sema;

StmtResult
Sema::ActOnSwarmSpawnStmt(SourceLocation SpawnLoc, Stmt *InitStmt,
                          ConditionResult Cond, Stmt *BodyStmt,
                          bool isSub, bool isSuper) {
  DiagnoseUnusedExprResult(BodyStmt);

  PushFunctionScope();
  // TODO: Figure out how to prevent jumps into and out of the spawned
  // substatement.
  getCurFunction()->setHasBranchProtectedScope();
  PushExpressionEvaluationContext(
      ExpressionEvaluationContext::PotentiallyEvaluated);

  SwarmSpawnStmt *SS;
  // All following code copied with modifications from
  // ActOnStartOfSwitchStmt() and ActOnFinishSwitchStmt()
  {
    if (Cond.isInvalid())
      return StmtError();

    //getCurFunction()->setHasBranchIntoScope();

    SS = new (Context)
        SwarmSpawnStmt(Context, SpawnLoc,
                       InitStmt, Cond.get().first, Cond.get().second,
                       BodyStmt,
                       isSub, isSuper);

    Expr *CondExpr = SS->getTimestamp();
    if (!CondExpr) return StmtError();

    QualType CondType = CondExpr->getType();

    // C++ 6.4.2.p2:
    // Integral promotions are performed (on the switch condition).
    //
    // A case value unrepresentable by the original switch condition
    // type (before the promotion) doesn't make sense, even when it can
    // be represented by the promoted type.  Therefore we need to find
    // the pre-promotion type of the switch condition.
    if (!CondExpr->isTypeDependent()) {
      // We have already converted the expression to an integral or enumeration
      // type, when we started the switch statement. If we don't have an
      // appropriate type now, just return an error.
      if (!CondType->isIntegralOrEnumerationType())
        return StmtError();

      if (CondExpr->isKnownToHaveBooleanValue()) {
        // switch(bool_expr) {...} is often a programmer error, e.g.
        //   switch(n && mask) { ... }  // Doh - should be "n & mask".
        // One can always use an if statement instead of switch(bool_expr).
        Diag(SpawnLoc, diag::warn_bool_switch_condition)
            << CondExpr->getSourceRange();
      }
    }

    if (BodyStmt)
      DiagnoseEmptyStmtBody(CondExpr->getLocEnd(), BodyStmt,
                            diag::warn_empty_swarm_spawn_body);
  }

  PopExpressionEvaluationContext();
  PopFunctionScopeInfo();

  return SS;
}

StmtResult Sema::ActOnSwarmSpawnStmt(SourceLocation SpawnLoc, Stmt *BodyStmt) {
  DiagnoseUnusedExprResult(BodyStmt);

  PushFunctionScope();
  // TODO: Figure out how to prevent jumps into and out of the spawned
  // substatement.
  getCurFunction()->setHasBranchProtectedScope();
  PushExpressionEvaluationContext(
      ExpressionEvaluationContext::PotentiallyEvaluated);

  SwarmSpawnStmt *SS = new (Context) SwarmSpawnStmt(SpawnLoc, BodyStmt);

  PopExpressionEvaluationContext();
  PopFunctionScopeInfo();

  return SS;
}

//TODO(victory):
//StmtResult
//Sema::ActOnSwarmForStmt(SourceLocation SwarmForLoc, SourceLocation LParenLoc,
//                       Stmt *First, ConditionResult Second, FullExprArg Third,
//                       SourceLocation RParenLoc, Stmt *Body) {
