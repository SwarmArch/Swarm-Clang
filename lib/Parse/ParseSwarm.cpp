//===--- ParseSwarm.cpp - Swarm Parsing -----------------------------------===//
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
//  This file implements the Swarm portions of the Parser interface.
//
//===----------------------------------------------------------------------===//

#include "clang/Parse/RAIIObjectsForParser.h"
#include "clang/Parse/Parser.h"

using namespace clang;

/// ParseSwarmSpawnStatement
///       jump-statement:
///         '_Swarm_spawn' statement
///         '_Swarm_spawn' '(' expression ')' statement
StmtResult Parser::ParseSwarmSpawnStatement(SourceLocation* TrailingElseLoc) {
  bool isSub = Tok.is(tok::kw__Swarm_spawn_sub);
  bool isSuper = Tok.is(tok::kw__Swarm_spawn_super);
  assert((isSub || isSuper || Tok.is(tok::kw__Swarm_spawn)) && "Not a _Swarm_spawn stmt!");
  SourceLocation SpawnLoc = ConsumeToken();  // eat the '_Swarm_spawn'.

  if (Tok.isNot(tok::l_paren)) {  // If there is no timestamp expression
    if(isSub || isSuper) {
      Diag(Tok, diag::err_domain_spawn_without_timestamp)
          << (isSub ? tok::kw__Swarm_spawn_sub : tok::kw__Swarm_spawn_super);
      SkipUntil(tok::semi);
      return StmtError();
    }

    // Copied from ParseCilkSpawnStatement
    StmtResult SubStmt = ParseStatement();
    if (SubStmt.isInvalid()) {
      SkipUntil(tok::semi);
      return StmtError();
    }
    return Actions.ActOnSwarmSpawnStmt(SpawnLoc, SubStmt.get());
  } else {  // If there is a timestamp expression
    // All following code copied with modification from ParseSwitchStatement()

    bool C99orCXX = getLangOpts().C99 || getLangOpts().CPlusPlus;

    // C99 6.8.4p3 - In C99, the switch statement is a block.  This is
    // not the case for C90.  Start the switch scope.
    //
    // C++ 6.4p3:
    // A name introduced by a declaration in a condition is in scope from its
    // point of declaration until the end of the substatements controlled by the
    // condition.
    // C++ 3.3.2p4:
    // Names declared in the for-init-statement, and in the condition of if,
    // while, for, and switch statements are local to the if, while, for, or
    // switch statement (including the controlled statement).
    //
    unsigned ScopeFlags = Scope::SwitchScope;
    if (C99orCXX)
      ScopeFlags |= Scope::DeclScope | Scope::ControlScope;
    ParseScope SwitchScope(this, ScopeFlags);

    // Parse the condition.
    StmtResult InitStmt;
    Sema::ConditionResult Cond;
    if (ParseParenExprOrCondition(&InitStmt, Cond, SpawnLoc,
                                  Sema::ConditionKind::Switch))
      return StmtError();

    // C99 6.8.4p3 - In C99, the body of the switch statement is a scope, even if
    // there is no compound stmt.  C90 does not have this clause.  We only do this
    // if the body isn't a compound statement to avoid push/pop in common cases.
    //
    // C++ 6.4p1:
    // The substatement in a selection-statement (each substatement, in the else
    // form of the if statement) implicitly defines a local scope.
    //
    // See comments in ParseIfStatement for why we create a scope for the
    // condition and a new scope for substatement in C++.
    //
    getCurScope()->AddFlags(Scope::BreakScope);
    ParseScope InnerScope(this, Scope::DeclScope, C99orCXX, Tok.is(tok::l_brace));

    // We have incremented the mangling number for the SwitchScope and the
    // InnerScope, which is one too many.
    if (C99orCXX)
      getCurScope()->decrementMSManglingNumber();

    // Read the body statement.
    StmtResult Body(ParseStatement(TrailingElseLoc));

    // Pop the scopes.
    InnerScope.Exit();
    SwitchScope.Exit();

    return Actions.ActOnSwarmSpawnStmt(SpawnLoc, InitStmt.get(), Cond, Body.get(), isSub, isSuper);
  }
}

//TODO(victory): StmtResult Parser::ParseSwarmForStatement(SourceLocation *TrailingElseLoc) {
