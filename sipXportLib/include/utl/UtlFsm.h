//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef UTLFSM_H
#define UTLFSM_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsSysLog.h"

// DEFINES
//#define LOG_STATE_CHANGES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

   /**
    * Module implementing state-machine algorithms, based on the
    * "State pattern" of _Design Patterns_ by Gamma et al., with the
    * addition of hierarchical states with entry and exit actions in
    * the manner of UML.
    *
    * \par
    * The approach is inspired by the State pattern described in GoF
    * (_Design Patterns_ by Gamma et al.), which is also described
    * briefly in http://en.wikipedia.org/wiki/State_pattern.
    *
    * \par
    * In GoF's pattern, a state machine is implemented by an object of
    * a Context class, which contains a pointer to a State object.
    * The State class is abstract, and has subclasses that represent
    * individual states.  Thus, a Context object actually points to a
    * instance of a subclass of State.  The fact of the state machine
    * being in a particular state is represented by the Context object
    * pointing to a state object of the appropriate subclass.  In the
    * GoF pattern, state transitions are implemented by deleting the
    * current instance object and replacing it with a newly-created
    * instance object of the correct subclass of State.  Various features
    * of C++ (especially std::auto_ptr) are exploited to simplify and
    * factor the code that performs these operations.
    *
    * \par
    * Instead of creating and destroying State objects at each transition,
    * this implementation requires that there is a only one instance
    * of each State subclass for each Context object.  The instances are
    * reused whenever the state machine enters the corresponding state.
    * When implementing hierarchical state machines, this makes processing
    * more efficient.  Details of this alternative are given below.
    *
    * \par
    * The Context object delegates state-dependent operations to the
    * State object.  In the code in the references above, the State
    * class has a protected operation called ChangeState, which is
    * inherited by its subclasses, and is called to execute a transition.
    * The State's ChangeState operation simply asks the Context object to
    * change the pointer.
    *
    * \par
    * The GoF pattern only works for flat state machines. In a
    * hierarchical state machine, states may contain substates, and
    * each state may have an entry action and an exit action.  A
    * transition may originate from, or terminate at, either a
    * composite state (which has substates) or a simple state.  The
    * implementation provided here includes a ChangeState operation
    * that calls all the exit and entry actions that need to be called
    * for the transition from any one state to any other.
    *
    * \par
    * In our implementation, Context class parameters of templates are
    * given as "class StateMachineType", and State class parameters
    * are given as "class StateType".  The method
    * <Context>::ChangeState() is usually implemented using
    * StateAlg::ChangeState(), as the processing needed to support
    * entry and exit actions makes ChangeState() more complicated than
    * the assignment (to a std::auto_ptr) that is used in the GoF
    * pattern.
    *
    * \par
    * To implement hierarchical state machines, we impose some
    * requirements on the Context and State classes.  The Context
    * class must implement methods GetCurrentState() and
    * SetCurrentState() to get and set the pointer to the current
    * state. The State class must implement operations called
    * DoEntryAction(), DoExitAction(), GetParent(), and
    * GetInitialState(), which will be inherited but re-implemented by
    * every subclass.  These methods (which are specified in more
    * detail below) are used by ChangeState(), which is defined in
    * this module.  In order to make working with hierarchical states
    * efficient, this implementation requires that there is a only one
    * instance of each State subclass for each Context object (and
    * usually one State subclass instance that is utilized by all
    * Context objects).
    *
    * \par
    * For logging, Context and State classes *must* implement a member
    * function called name() that returns a C-style string containing
    * the name of the Context instance or State class.  This method is
    * only used if the manifest constant LOG_STATE_CHANGES is defined
    * at compile time.
    *
    * \par
    * Typically, substates will be implemented as subclasses of the
    * parent state. This means that substates do not need to re-implement
    * events that are handled by the parent class; i.e., substates
    * inherit behaviour. This corresponds to the way in which transitions
    * are inherited in UML state machines.
    *
    * \par
    * Our implementation does not mention transition actions or guards
    * or internal transitions, since these are simply coded in the methods
    * of the concrete subclasses of State.  Note that, to perform an
    * external transition, these methods will perform the transition
    * action and then call ChangeState, which will perform the exit and
    * entry actions. This is a slight divergence
    * from UML, since the exit actions should be performed before
    * the transition action, but it seldom matters in practice.
    * This was also the approach taken in the book by Samek.
    *
    * \par
    * In this module, each function is a template function taking two
    * types parameters:  StateMachineType (corresponding to Context in
    * the GoF pattern) and StateType (corresponding to State in the
    * GoF pattern).
    *
    * StateMachineType (corresponding to Context in the GoF pattern) must be
    * a class that defines the following methods:
    *
    * \code
    *  // Returns the current state, or 0 if there isn't one.
    *  // (Zero can only be returned if the state machine hasn't started yet.)
    *  //
    *  StateType* GetCurrentState() const;
    *
    *  // Sets the current state.
    *  //
    *  void SetCurrentState( StateType* state );
    * \endcode
    *
    * StateType must be a class that defines the following methods:
    *
    * \code
    *  // Performs the entry action for the state.
    *  //
    *  // Note:
    *  //    The entry action of a composite state must not call ChangeState.
    *  //    The entry action of a simple state may call ChangeState (this
    *  //    corresponds to a completion transition; i.e., a transition without
    *  //    an event).
    *  //
    *  virtual void DoEntryAction( StateMachineType& sm );
    *
    *  // Performs the exit action for the state.
    *  //
    *  // Exit actions must not call ChangeState.
    *  //
    *  virtual void DoExitAction( StateMachineType& sm );
    *
    *  // Returns the parent of this state, or 0 if there is no parent.
    *  //
    *  // When performing the entry and exit actions, the implementation
    *  // compares pointers to State objects. The assumption is that there
    *  // will be one object to represent each state in the state machine.
    *  // If A and B both have the same parent state, then A->GetParent()
    *  // must return the same pointer as B->GetParent().
    *  //
    *  virtual StateType* GetParent( StateType& sm ) const;
    *
    *  // Returns the initial state contained in this state,
    *  // or 0 if there isn't one. A state that contains no other
    *  // states must return 0. A state that contains other states
    *  // and that is the target of a transition must not return 0.
    *  //
    *  // The initial state must be a direct descendant of this state.
    *  //
    *  virtual StateType* GetInitialState( StateType& sm ) const;
    * \endcode
    *
    * \par
    * If the states have no data, you can declare all their member functions
    * const (including DoEntryAction and DoExitAction) and declare
    * GetCurrentState to return a const StateType* and declare
    * SetCurrentState to take a const StateType* parameter.
    *
    * \par
    * All of the StateType methods take a StateMachineType& parameter.
    * This is so that states do not need to store any data, so that multiple
    * instances of a state machine can share the same set of state objects.
    *
    * \par
    * Care must be taken with callbacks made from the state machine. There are
    * two issues:
    * 1. The callback may invoke a method that is handled by the state machine.
    *    Therefore, you must ensure that the state machine is in the correct
    *    state before making the callback. Typically, this means calling
    *    ChangeState before making the callback. (And typically the call
    *    to ChangeState will be the last thing done except for the callback.)
    *    Invoking a callback from an entry action does not have this issue, because
    *    ChangeState sets the new current state before invoking the entry
    *    actions (* - more details below). The current state is not changed before
    *    exit actions are invoked, so the current state while exit actions are
    *    executing is the old state. Note that the reason this is an issue is that
    *    the state machines implemented as described herein do not have run-
    *    to-completion semantics; i.e., they do not ensure that an event is
    *    completely handled before starting to process the next event. You
    *    could implement a run-to-completion scheme by enqueuing events, but
    *    not without overhead.
    * 2. The callback may delete the Context object. I believe this will not be
    *    a problem if the callback is always the last thing done in the State's
    *    member function (and if callbacks are not made from exit actions or
    *    from the entry actions of composite states). (TO DO: Test this.) It
    *    is also possible to devise schemes that delay deletion of the context
    *    class until all callbacks have returned.
    *
    *    \par
    *    (*) Some more details on the current state during a state transition:
    *    In a transition from A to B, the state machine algorithm finds the "most
    *    nested initial state" of B. Call this I. If B is a simple state, then
    *    I = B. Otherwise, I = B->GetInitialState(sm); if this state has an initial
    *    state, then I = I->GetInitialState(sm), until a simple state is reached.
    *    While the exit actions of A's children (as required), of A, and of
    *    A's parents (as required) are performed, the current state remains
    *    unchanged. Then the current state is changed to I, and the entry actions
    *    of I's parents (as required) and of I are performed. Now, I's parents'
    *    entry actions are not allowed to call ChangeState, but I's entry action
    *    may call ChangeState. Suppose I's entry action does that, and the new
    *    state is C. Let J be the most nested initial state of C. ChangeState
    *    will perform the exit actions of I and of I's parents (as required),
    *    then set the current state to J, then call the entry actions of
    *    J's parents (as required) and of J.
    **/
   class StateAlg
   {
   public:
      //----------------------- ChangeState ---------------------------
      /**
       * Changes from the current state to the transition target
       * state, performing the required exit and entry
       * actions. Self-transitions, as well as transitions to ancestor
       * or descendant states, are supported.  Client code (in state
       * classes) calls this function to perform external
       * transitions. Recall that events are handled by methods in the
       * StateMachineType (Context) class that delegate to methods in
       * the StateType (State) classes. When an event results in an
       * external transition, the method in the State class calls this
       * function (ChangeState) to effect the transition. In addition,
       * the entry actions of simple states may call this
       * function. (The entry actions of composite states must not
       * call this function.)
       *
       * \note
       * When you call this template function, make sure the StateType*
       * parameters are of the right type. (Otherwise the compiler will
       * complain.) For example, suppose your base
       * class is State, and suppose you have subclasses of State called
       * State1, State2, and State3. Make sure the parameters are of type
       * State* and not State1*, State2* or State3*. One way to do this
       * is to define variables, as follows:
       * \code
       *    State* source = this;
       *    State* target = targetState;
       *    ChangeState( sm, source, target);
       * \endcode
       * It is probably best to call ChangeState in one place, in a
       * method in the State class, that the subclasses can call
       * without defining the extra variables; for example:
       * \code
       * void State::ChangeState( StateMachine& sm, State* targetState )
       * {
       *    StateAlg::ChangeState( sm, this, targetState );
       * }
       * \endcode
       * Now any method in State1 (or State2 or State3) can just call ChangeState:
       * \code
       *    ChangeState( sm, pState2 );
       * \endcode
       * (Note, though, that when an action is inherited, calling ChangeState as
       * shown changes the source state of the transition.  If it matters, the
       * source state can be explicitly specified.)
       * TO DO: A version of ChangeState that takes const state
       * parameters (see private_sigchannelstate.h)
       * You can also choose to get rid of the "sm" parameter of the local
       * ChangeState method, by making it a member
       * of every state class. The disadvantage of doing so is that every state
       * class becomes tied to a specific StateMachine instance, so if there are
       * multiple StateMachine instances, there also have to be multiple instances
       * of each State class.
       *
       * \par
       * See the documentation for class StateAlg for more details.
       */
      template< class StateMachineType, class StateType >
      static void ChangeState( StateMachineType& sm,
                               StateType* transitionSource,
                               StateType* transitionTarget )
      {
#ifdef LOG_STATE_CHANGES
         OsSysLog::add(FAC_FSM, PRI_DEBUG,
                       "ChangeState( FSM = %s, current state = %s. new state = %s )",
                       sm.name(),
                       transitionSource->name(),
                       transitionTarget->name() );
#endif
         Exit( sm, transitionSource );
         ParentExit( sm, transitionSource->GetParent( sm ), transitionTarget );
         sm.SetCurrentState( GetMostNestedInitialState( sm, transitionTarget ) );
         ParentEnter( sm, transitionTarget->GetParent( sm ), transitionSource );
         Enter( sm, transitionTarget );
      }

      //---------------------- StartStateMachine ----------------------
      /**
       * Enters the state machine's initial state. This is called once
       * for each created state machine instance, to start the state
       * machine.  The initial state must be a top-level state; i.e.,
       * it must not have a parent.
       *
       * \par
       * The state's own entry action is performed first. If the state
       * is composite, its initial state is then entered (and if the
       * state's initial state is composite, the process continues
       * until a simple state is reached).
       *
       * \par
       * See the note with ChangeState() about making sure the
       * stateToEnter parameter is of the correct type. See the
       * documentation with StateAlg for more details about state
       * machines.
       *
       * \param stateToEnter
       *    The state machine's initial state.
       */
      template< class StateMachineType, class StateType >
      static void StartStateMachine( StateMachineType& sm, StateType* stateToEnter )
      {
         sm.SetCurrentState( GetMostNestedInitialState( sm, stateToEnter ) );
         Enter( sm, stateToEnter );
      }

   private:
      /**
       * Performs the exit action for the current state and all of its
       * ancestor states up to and including state transitionSource
       * (which is expected to be the state which is the source of a
       * transition).  In other words, exits the current state, the
       * parent of the current state, the parent's parent, etc., until
       * transitionSource has been exited.
       *
       * \note
       * This function assumes that transitionSource is either the
       * current state or an ancestor of the current state, which
       * will be the case in a well-formed state machine.
       */
      template< class StateMachineType, class StateType >
      static void Exit( StateMachineType& sm, StateType* transitionSource )
      {
         StateType* current = sm.GetCurrentState();
         StateType* prev;

         do
         {
            current->DoExitAction( sm );
            prev = current;
            current = current->GetParent( sm );
         }
         while ( prev != transitionSource && current != 0 );
      }  //lint !e818

      /**
       * Exits from the parent of transitionSource and all of its
       * ancestors that are exited when transitioning to state
       * transitionTarget.
       *
       * \param parentToExit The parent or ancestor of the transition
       *    source that may need to be exited. This parameter is
       *    allowed to be 0, in which case nothing is done.
       * \param transitionTarget The target of the transition. The
       *    parameter is needed because it determines which ancestors
       *    need to be exited:  those ancestors of transitionSource
       *    which are not also ancestors of transitionTarget.
       */
      template< class StateMachineType, class StateType >
      static void ParentExit( StateMachineType& sm, StateType* parentToExit, StateType* transitionTarget )
      {
         if ( parentToExit == 0 )
            return;

         if ( parentToExit == transitionTarget )
         {
            parentToExit->DoExitAction( sm );
            return;
         }

         if ( IsAncestor( sm, parentToExit, transitionTarget ) )
         {
            return;
         }

         parentToExit->DoExitAction( sm );
         ParentExit( sm, parentToExit->GetParent( sm ), transitionTarget );
      }

      /**
       * Performs the entry action of the parent (parentToEnter) of a
       * state, as well as the entry actions of the parent's ancestors
       * that are entered when transitioning from state
       * transitionSource.
       */
      template< class StateMachineType, class StateType >
      static void ParentEnter( StateMachineType& sm, StateType* parentToEnter, StateType* transitionSource )
      {
         if ( parentToEnter == 0 )
            return;

         if ( parentToEnter == transitionSource )
         {
            parentToEnter->DoEntryAction( sm );
            return;
         }

         if ( IsAncestor( sm, parentToEnter, transitionSource ) )
         {
            return;
         }

         ParentEnter( sm, parentToEnter->GetParent( sm ), transitionSource );
         parentToEnter->DoEntryAction( sm );
      }

      /**
       * Enters a state, recursively entering initial substates of
       * composite states.
       * Precondition: The new current state has already been set.
       *
       * \param stateToEnter Either the target of a transition or an
       *    initial state contained (either directly or indirectly)
       *    within the target of a transition.
       */
      template< class StateMachineType, class StateType >
      static void Enter( StateMachineType& sm, StateType* stateToEnter )
      {
         StateType* initialState = stateToEnter->GetInitialState( sm );

         if ( initialState == 0 )
         {
            stateToEnter->DoEntryAction( sm );
         }
         else
         {
            stateToEnter->DoEntryAction( sm );
            Enter( sm, initialState );
         }
      }

      /**
       * Determines whether one state is the ancestor of another.
       * A state is not considered its own ancestor.
       *
       * /note
       *    The StateType parameters are not declared const so that the client
       *    can have GetParent and GetInitialState return const StateType*
       *    and be able to call ChangeState with either StateType* or
       *    const StateType*.
       */
      template< class StateMachineType, class StateType >
      static bool IsAncestor( StateMachineType& sm, StateType* possibleAncestor, StateType* possibleDescendant )
      {

         if ( possibleAncestor == 0 )
            return false;

         StateType* parent = possibleDescendant->GetParent( sm );

         while ( parent != 0 )
         {
            if ( parent == possibleAncestor )
               return true;
            parent = parent->GetParent( sm );
         }

         return false;
      }

      /**
       * If startState is a simple state, returns
       * startState. Otherwise, recursively examines the initial state
       * of complex states.  That is, it returns startState's initial
       * state if that is a simple state, or startState's initial
       * state's initial state if that is a simple state, etc.
       */
      template< class StateMachineType, class StateType >
      static StateType* GetMostNestedInitialState( StateMachineType& sm, StateType* startState )
      {
         StateType* initialState = startState->GetInitialState( sm );
         if ( initialState )
            return GetMostNestedInitialState( sm, initialState );
         else
            return startState;
      }

   };

#endif  // #ifndef UTLFSM_H
