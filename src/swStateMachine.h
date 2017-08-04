
//
//  Finite state machine.
//
//  Copyright (c) 2007 Waync Cheng.
//  All Rights Reserved.
//
//  2007/07/23 Waync created.
//

///
/// \file
/// \brief Finite state machine.
///
/// State machine module provides a general automatic state management, application
/// can create state transition rules easily. FSM usually uses in game AI, also
/// can be used in any application that states changes. Ex: game object animation,
/// flow control, network state control, etc.
///
/// Example:
///
/// Following is a simple game sample: there are 3 states(menu, gameplay, quit),
/// 2 inputs(esc, enter).
///
/// - menu: switch to quit if input esc(rule 1), switch to gamepay if input enter(rule 2).
/// - gameplay: switch to menu state if input esc(rule 3).
/// - quit: not accept any input.
///
/// \code
/// #include "StateMachine.h"
///
/// //
/// // Define states and input.
/// //
///
/// enum STATES { menu, gameplay, quit }; // Define state(can be any type, ex: string)
/// enum INPUTS { esc, enter };         // Define input state(can be any type, ex: string)
///
/// //
/// // Declare a state machine.
/// //
///
/// StateMachine<STATES, INPUTS> states;
///
/// //
/// // Create transition rules.
/// //
///
/// states.addTransition(menu, esc, quit); // (rule 1)
/// states.addTransition(menu, enter, gameplay); // (rule 2)
/// states.addTransition(gameplay, esc, menu); // (rule 3)
///
/// //
/// // Handle state and input.
/// //
///
/// STATES curr = menu;                 // Current game state.
/// INPUTS inp = getInput();            // Get current input.
///
/// STATES next;
/// if (states.input(curr, inp, next))  // Check is any state change.
/// { // YES, current state is changed to next state.
///    curr = next;
/// }
/// else
/// { // NO, current state doesn't accept this input state, no state chnage.
/// }
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2007/07/23
///

#pragma once

#include <map>

#include "swinc.h"

namespace sw2 {

namespace impl {

template<typename T>
struct implStateMachineTraits
{
  bool operator()(T const& a, T const& b) const
  {
    if (a.first > b.first) {
      return false;
    } else if (a.first < b.first) {
      return true;
    }

    if (a.second > b.second) {
      return false;
    } else if (a.second < b.second) {
      return true;
    }

    return false;
  }
};

} // namespace impl

///
/// \brief Finite state machine.
///

template<typename StateT, typename InputT>
class StateMachine
{
public:

  typedef std::pair<StateT, InputT> InputSet;
  typedef StateT OutputT;
  typedef impl::implStateMachineTraits<InputSet> TraitsT;

  typedef typename std::map<InputSet, OutputT, TraitsT>::iterator iterator;
  typedef typename std::map<InputSet, OutputT, TraitsT>::const_iterator const_iterator;

  std::map<InputSet, OutputT, TraitsT> transitions; // (current state, an input) -> (next state)

  ///
  /// \brief Add a new transition rule.
  /// \param [in] state Current state.
  /// \param [in] input Input state.
  /// \param [in] output Output state of (Current state, Input state).
  /// \return Return true if success else return false.
  /// \note Different state or input may map to the same output state.
  ///

  bool addTransition(StateT const& state, InputT const& input, OutputT const& output)
  {
    InputSet i(state, input);
    if (transitions.end() != transitions.find(i)) {
      return false;
    }

    transitions[i] = output;

    return true;
  }

  ///
  /// \brief Remove a transition rule.
  /// \param [in] state Current state.
  /// \param [in] input Input state.
  /// \return Return true if success else return false.
  ///

  bool removeTransition(StateT const& state, InputT const& input)
  {
    InputSet i(state, input);

    iterator it = transitions.find(i);
    if (transitions.end() == it) {
      return false;
    }

    transitions.erase(it);

    return true;
  }

  ///
  /// \brief Get next state of (Current state, Input state).
  /// \param [in] state Current state.
  /// \param [in] input Next state.
  /// \param [out] output Output state of (Current state, Input state).
  /// \return Return true if exists an output state of (Current state, Input state)
  ///         and output is the output state of (Current state, Input state),
  ///         else return false.
  ///

  bool input(StateT const& state, InputT const& input, OutputT& output) const
  {
    InputSet i(state, input);

    const_iterator it = transitions.find(i);
    if (transitions.end() == it) {
      return false;
    }

    output = it->second;

    return true;
  }

  ///
  /// \brief Get next state of (Current state, Input state).
  /// \param [in] state Current state.
  /// \param [in] input Next state.
  /// \param [out] output Output state of (Current state, Input state).
  /// \param [in] cond Additional use define rule.
  /// \return Return true if exists an output state of (Current state, Input state)
  ///         and output is the output state of (Current state, Input state),
  ///         else return false.
  /// \note ConT is a functor or function pointer, prototype is:
  ///       bool operator()(StateT const& state, InputT const& input);
  ///

  template<typename CondT>
  bool input(StateT const& state, InputT const& input, OutputT& output, CondT& cond) const
  {
    InputSet i(state, input);

    const_iterator it = transitions.find(i);
    if (transitions.end() == it) {
      return false;
    }

    if (!cond(state, input)) {
      return false;
    }

    output = it->second;

    return true;
  }
};

} // namespace sw2

// end of swStateMachine.h
