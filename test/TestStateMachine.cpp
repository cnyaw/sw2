
//
//  StateMachine unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/26 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swStateMachine.h"
using namespace sw2;

//
// Test1,use std input.
//

TEST(StateMachine, test)
{
  //
  // Define state and input.
  //

  int const menu = 0, game = 1, quit = 2; // Define state(Can be any type, ex:string)
  int const esc = 0, enter = 1;         // Define input(Can be any type, ex:string)

  //
  // State machine.
  //

  StateMachine<int,int> states;

  //
  // Build state transition rules.
  //

  states.addTransition(menu, esc, quit); // (Rule1)
  states.addTransition(menu, enter, game); // (Rule12)
  states.addTransition(game, esc, menu); // (Rule13)

  //
  // Start test.
  //

  int next;

  CHECK(states.input(menu, esc, next) && quit == next);
  CHECK(states.input(menu, enter, next) && game == next);

  CHECK(states.input(game, esc, next) && menu == next);
  CHECK(!states.input(game, enter, next));

  CHECK(!states.input(quit, esc, next));
  CHECK(!states.input(quit, enter, next));
}

// end of TestStateMachine.cpp
