#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <string.h>
#include <stdlib.h>
#include <cstdarg>
#include <time.h>
#include <os/OsDefs.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlFsm.h>

using namespace std;

class State;

string StateChanges;


class Context
{
public:
   Context() : m_state(0) {}
   State* GetCurrentState() const { return m_state; }  //lint !e1763
   void SetCurrentState( State* state ) { m_state = state; }
   const char* name( void ) const
   {
      return "Context";
   }

private:
   State* m_state;
};

class State
{
public:
   State() :
      m_parent( 0 ),
      m_initialState( 0 ),
      m_entryString( "" ),
      m_exitString( "" ),
      m_entryTransitionsTo( 0 )
   {
   }

   State( State* parent, State* initialState, const string& entryString, const string& exitString,
          State* nextState = 0 ) :
      m_parent( parent ),
      m_initialState( initialState ),
      m_entryString( entryString ),
      m_exitString( exitString ),
      m_entryTransitionsTo( nextState )
   {
   }

   virtual ~State()
   {
   }

   const char* name( void ) const
   {
      return "State";
   }

   virtual void DoEntryAction( Context& context )
   {
      StateChanges += m_entryString;
      StateChanges += " ";
      if ( m_entryTransitionsTo != 0 )
         StateAlg::ChangeState( context, this, m_entryTransitionsTo );
   }

   virtual void DoExitAction( Context& context )
   {
      StateChanges += m_exitString;
      StateChanges += " ";
   }

   virtual State* GetParent( Context& context ) const
   {
      return m_parent;
   }  //lint !e1763

   virtual State* GetInitialState( Context& context ) const
   {
      return m_initialState;
   }  //lint !e1763

private:
   State* m_parent;
   State* m_initialState;
   string m_entryString;
   string m_exitString;
   State* m_entryTransitionsTo;
};

class UtlFsmTest : public  CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UtlFsmTest);
    CPPUNIT_TEST( testTransitionFromA1aToA1a );
    CPPUNIT_TEST( testTransitionFromA1aToA1b );
    CPPUNIT_TEST( testTransitionFromA1aToA1  );
    CPPUNIT_TEST( testTransitionFromA1aToA   );
    CPPUNIT_TEST( testTransitionFromA1aToA2  );
    CPPUNIT_TEST( testTransitionFromA1aToB   );
    CPPUNIT_TEST( testTransitionFromA1bToA   );
    CPPUNIT_TEST( testTransitionFromA1ToA1a  );
    CPPUNIT_TEST( testTransitionFromA1ToA1b  );
    CPPUNIT_TEST( testTransitionFromA1ToA1   );
    CPPUNIT_TEST( testTransitionFromA1ToA    );
    CPPUNIT_TEST( testTransitionFromA1ToA2   );
    CPPUNIT_TEST( testTransitionFromA1ToB    );
    CPPUNIT_TEST( testTransitionFromA2ToA1a  );
    CPPUNIT_TEST( testTransitionFromA2ToA1b  );
    CPPUNIT_TEST( testTransitionFromA2ToA1   );
    CPPUNIT_TEST( testTransitionFromA2ToA    );
    CPPUNIT_TEST( testTransitionFromA2ToA2   );
    CPPUNIT_TEST( testTransitionFromA2ToB    );
    CPPUNIT_TEST( testTransitionFromAToA1a   );
    CPPUNIT_TEST( testTransitionFromAToA1b   );
    CPPUNIT_TEST( testTransitionFromAToA1    );
    CPPUNIT_TEST( testTransitionFromAToA     );
    CPPUNIT_TEST( testTransitionFromAToA2    );
    CPPUNIT_TEST( testTransitionFromAToB     );
    CPPUNIT_TEST( testTransitionFromBToA1a   );
    CPPUNIT_TEST( testTransitionFromBToA1b   );
    CPPUNIT_TEST( testTransitionFromBToA1    );
    CPPUNIT_TEST( testTransitionFromBToA     );
    CPPUNIT_TEST( testTransitionFromBToA2    );
    CPPUNIT_TEST( testTransitionFromBToB     );
    CPPUNIT_TEST( testTransitionsInEntryActions );
    CPPUNIT_TEST( testStartStateMachineAtCompositeState );
    CPPUNIT_TEST( testStartStateMachineAtSimpleState );
    CPPUNIT_TEST( testStartStateMachineAtCompositeStateFollowedByEntryAction );
    CPPUNIT_TEST_SUITE_END();

private:
   Context context;

   State stateA;
   State stateA1;
   State stateA2;
   State stateA1a;
   State stateA1b;
   State stateB;

   State* const A;
   State* const A1;
   State* const A2;
   State* const A1a;
   State* const A1b;
   State* const B;

   //  ---------------------------------
   //  |               A               |
   //  |                               |
   //  | ---------------------         |
   //  | |        A1         |         |
   //  | |                   |  ------ |    ------
   //  | | -------   ------- |  | A2 | |    | B  |
   //  | | | A1a |   | A1b | |  ------ |    ------
   //  | | -------   ------- |         |
   //  | ---------------------         |
   //  |                               |
   //  ---------------------------------

public:
   UtlFsmTest() :
        A  ( &stateA ),
        A1 ( &stateA1 ),
        A2 ( &stateA2 ),
        A1a( &stateA1a ),
        A1b( &stateA1b ),
        B  ( &stateB ){}


   void setUp()
   {
      StateChanges = "";
      context.SetCurrentState( 0 );

      stateA   = State( 0,  A1,  "A",   "xA" );
      stateA1  = State( A,  A1a, "A1",  "xA1" );
      stateA2  = State( A,  0,   "A2",  "xA2" );
      stateA1a = State( A1, 0,   "A1a", "xA1a" );
      stateA1b = State( A1, 0,   "A1b", "xA1b" );
      stateB   = State( 0,  0,   "B",   "xB" );
   }

   void tearDown()
   {
   }

   void testTransitionFromA1aToA1a()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, A1a );
      CPPUNIT_ASSERT( StateChanges == "xA1a A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1aToA1b()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, A1b );
      CPPUNIT_ASSERT( StateChanges == "xA1a A1b " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1b );
   }

   void testTransitionFromA1aToA1 ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, A1 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1aToA  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, A );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1aToA2 ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, A2 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A2 " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A2 );
   }

   void testTransitionFromA1aToB  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1a, B );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }


   void testTransitionFromA1bToA  ()
   {
      context.SetCurrentState( A1b );
      StateAlg::ChangeState( context, A1b, A );
      CPPUNIT_ASSERT( StateChanges == "xA1b xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }


   void testTransitionFromA1ToA1a ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, A1a );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1ToA1b ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, A1b );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A1 A1b " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1b );
   }

   void testTransitionFromA1ToA1  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, A1 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1ToA   ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, A );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA1ToA2  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, A2 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 A2 " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A2 );
   }

   void testTransitionFromA1ToB   ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A1, B );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }


   void testTransitionFromA2ToA1a ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, A1a );
      CPPUNIT_ASSERT( StateChanges == "xA2 A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA2ToA1b ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, A1b );
      CPPUNIT_ASSERT( StateChanges == "xA2 A1 A1b " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1b );
   }

   void testTransitionFromA2ToA1  ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, A1 );
      CPPUNIT_ASSERT( StateChanges == "xA2 A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA2ToA   ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, A );
      CPPUNIT_ASSERT( StateChanges == "xA2 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromA2ToA2  ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, A2 );
      CPPUNIT_ASSERT( StateChanges == "xA2 A2 " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A2 );
   }

   void testTransitionFromA2ToB   ()
   {
      context.SetCurrentState( A2 );
      StateAlg::ChangeState( context, A2, B );
      CPPUNIT_ASSERT( StateChanges == "xA2 xA B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }


   void testTransitionFromAToA1a  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, A1a );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromAToA1b  ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, A1b );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1b " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1b );
   }

   void testTransitionFromAToA1   ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, A1 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromAToA    ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, A );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromAToA2   ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, A2 );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA A A2 " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A2 );
   }

   void testTransitionFromAToB    ()
   {
      context.SetCurrentState( A1a );
      StateAlg::ChangeState( context, A, B );
      CPPUNIT_ASSERT( StateChanges == "xA1a xA1 xA B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }

   void testTransitionFromBToA1a  ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, A1a );
      CPPUNIT_ASSERT( StateChanges == "xB A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromBToA1b  ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, A1b );
      CPPUNIT_ASSERT( StateChanges == "xB A A1 A1b " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1b );
   }

   void testTransitionFromBToA1   ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, A1 );
      CPPUNIT_ASSERT( StateChanges == "xB A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromBToA    ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, A );
      CPPUNIT_ASSERT( StateChanges == "xB A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testTransitionFromBToA2   ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, A2 );
      CPPUNIT_ASSERT( StateChanges == "xB A A2 " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A2 );
   }

   void testTransitionFromBToB    ()
   {
      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, B );
      CPPUNIT_ASSERT( StateChanges == "xB B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }

   void testTransitionsInEntryActions()
   {
      State stateC;
      State stateD;
      State* C = &stateC;
      State* D = &stateD;
      stateC = State( 0,  0, "C", "xC", D );
      stateD = State( 0,  0, "D", "xD", A1 );

      context.SetCurrentState( B );
      StateAlg::ChangeState( context, B, C );
      CPPUNIT_ASSERT( StateChanges == "xB C xC D xD A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testStartStateMachineAtCompositeState()
   {
      StateAlg::StartStateMachine( context, A );
      CPPUNIT_ASSERT( StateChanges == "A A1 A1a " );
      CPPUNIT_ASSERT( context.GetCurrentState() == A1a );
   }

   void testStartStateMachineAtSimpleState()
   {
      StateAlg::StartStateMachine( context, B );
      CPPUNIT_ASSERT( StateChanges == "B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }

   void testStartStateMachineAtCompositeStateFollowedByEntryAction()
   {
      stateA1a = State( A1, 0, "A1a", "xA1a", B );
      StateAlg::StartStateMachine( context, A );
      CPPUNIT_ASSERT( StateChanges == "A A1 A1a xA1a xA1 xA B " );
      CPPUNIT_ASSERT( context.GetCurrentState() == B );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlFsmTest);
