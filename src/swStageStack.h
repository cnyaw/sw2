
//
//  Program flow controller.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/25 Waync created.
//

///
/// \file
/// \brief Program flow controller.
///
/// Stage stack uses the stack mechanism to control the flow of program. It is
/// straightforward to treat the states of a program in stack.
///
/// For example, following sample has 3 state: MainMenu, GamePlay and QuitApp.
///
/// - MainMenu: the begining state, if user press ESC key then switch to QuitApp
///             state, if press Enter key then switch to GamePlay state.
/// - GamePlay: if press ESC key then return to MainMenu state.
/// - QuitApp: quit application.
///
/// The application shows when state is switched to GamePlay it can switch back to
/// MainMenu state, so the state GamePlay is push to the state stack on top of
/// MainMenu. And just pop the GamePlay from stack then the state is returned
/// to MainMenu again.
///
/// Example:
///
/// \code
/// #include "swStageStack.h"
///
/// class myClass
/// {
/// public:
///
///   void runGame()
///   {
///     //
///     // Initialize the state to MainMenu.
///     //
///
///     m_trigger.initialize(this, &myClass::stageMainMenu);
///     while (!bGameQuit)
///     {
///       m_trigger.trigger();          // Trigger the state machine.
///     }
///   }
///
/// private:
///
///   //
///   // Following 3 functions are correspond to 3 program states.
///   //
///
///   void stageMainMenu(int state, uint_ptr) // MainMenu state.
///   {
///     if (JOIN == state)
///     { // Do some initialization.
///       bQuitGame = false;
///     }
///     if (TRIGGER == state)
///     {
///       if (isESCKeyPressed()) {      // ESC: switch to QuitApp state.
///         m_trigger.popAndPush(&myClass::stageQuitApp);
///       } else if (isEnterKeyPressed()) {// Enter: switch GamePlay state.
///         m_trigger.popAndPush(&myClass::stageGamePlay);
///       }
///     }
///   }
///   void stageGamePlay(int state, uint_ptr) // GamePlay state.
///   {
///     if (JOIN == state)
///     { // Do some initialization.
///     }
///     if (TRIGGER == state)
///     {
///       if (isESCKeyPressed()) {      // ESC: switch to MainMenu state.
///         m_trigger.popAndPush(&myClass::stageMainMenu);
///       }
///     }
///     if (LEAVE == state)
///     { // Do some finialization.
///     }
///   }
///   void stageQuitApp(int state, uint_ptr) // QuitApp state.
///   {
///     if (JOIN == state)
///     {
///       bQuitGame = true;
///     }
///   }
///
/// private:
///
///   StageStack<myClass> m_trigger; // The controller.
/// };
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2005/02/25
///

#pragma once

#include "swinc.h"

namespace sw2 {

///
/// Stage state.
//

enum STAGESTACK_STATE
{
  JOIN,                                 ///< When a new state is pushsed, new state will get this state notify.
  LEAVE,                                ///< When a state is popped, previous state will get this state notify.
  TRIGGER,                              ///< When trigger the controller, current state will get this notify.
  SUSPEND,                              ///< When a new state is pushed, previous state will get this notify.
  RESUME                                ///< When a state is popped, previous state will get this notify.
};

///
/// \brief Stage stack.
///

template<class T, int MAX_STAGE = 8> class StageStack
{
public:

  ///
  /// \brief State function prototype.
  ///

  typedef void (T::*Stage)(int state, uint_ptr param);

  StageStack() : m_pHost(0), m_top(-1)
  {
  }

  ///
  /// \brief Change controller host.
  /// \param [in] pHost Controller host.
  /// \note A controller host is a class that hosts and processes the state
  ///       functions.
  ///

  void setHost(T* pHost)
  {
    assert(pHost);
    m_pHost = pHost;
  }

  ///
  /// \brief Initialize the controller.
  /// \param [in] pHost Controller host.
  /// \param [in] stage Initial state.
  ///

  void initialize(T* pHost, Stage stage)
  {
    assert(pHost && stage);
    m_top = -1;
    m_pHost = pHost;
    push(stage);
  }

  ///
  /// \brief Add and switch to new state.
  /// \param [in] stage New state.
  /// \note Flow controller is a stage stack, switch to a new state means push
  ///       the new state to the stack. Everytime executes a pop, the current
  ///       state is removed and switched to previous state.
  ///

  void push(Stage stage)
  {
    assert(stage && MAX_STAGE - 1 > m_top);
    if (-1 != m_top) {
      (m_pHost->*m_stack[m_top])(SUSPEND, 0);
    }
    m_stack[++m_top] = stage;
    (m_pHost->*stage)(JOIN, 0);
  }

  ///
  /// \brief Remove current state and switch to previous state.
  /// \param [in] popCount Pop count.
  ///

  void pop(int popCount = 1)
  {
    for (int i = 0; i < popCount; i++) {
      assert(-1 != m_top);
      int top = m_top;
      m_top -= 1;
      (m_pHost->*m_stack[top])(LEAVE, 0);
      if (-1 != m_top) {
        (m_pHost->*m_stack[m_top])(RESUME, 0);
      }
    }
  }

  ///
  /// \brief Do pop and push state at the same time.
  /// \param [in] stage New state.
  /// \param [in] popCount Pop count.
  ///

  void popAndPush(Stage stage, int popCount = 1)
  {
    pop(popCount);
    push(stage);
  }

  ///
  /// \brief Clear all states.
  ///

  void popAll()
  {
    assert(m_pHost);
    for (int i = m_top; i > -1; i--) {
      (m_pHost->*m_stack[i])(LEAVE, 0);
    }
    m_top = -1;
  }

  ///
  /// \brief Trigger the controller.
  /// \param [in] param User define data, pass to state function 2nd parameter.
  ///

  void trigger(uint_ptr param = 0)
  {
    assert(-1 != m_top);
    (m_pHost->*m_stack[m_top])(TRIGGER, param);
  }

  ///
  /// \brief Get current state.
  /// \return Return current state else return 0 if stack is empty.
  ///

  Stage top() const
  {
    if (-1 == m_top) {
      return 0;
    } else {
      return m_stack[m_top];
    }
  }

private:

  T* m_pHost;                           // Stage host.

  int m_top;                            // Current active stage.
  Stage m_stack[MAX_STAGE];             // Stage stack.
};

} // namespace sw2

// end of swStageStack.h
