//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtTerminalConnection_h_
#define _PtTerminalConnection_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtDefs.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtConnection;
class PtTerminal;
class TaoClientTask;
class TaoServerTask;
class TaoReference;
class TaoObjectMap;
class MpStreamPlayer;
class MpStreamPlaylistPlayer;


//:A PtTerminalConnection object represents the relationship between a
//:PtConnection and a PtTerminal.
// A PtTerminalConnection object must always be associated with a
// PtConnection object and a PtTerminal object. The association of a
// PtConnection and PtTerminal object to the PtTerminalConnection does not
// change throughout the lifetime of the PtTerminalConnection. Applications
// obtain the PtConnection and PtTerminal associated with the
// PtTerminalConnection via the PtTerminalConnection.getConnection() and
// PtTerminalConnection.getTerminal() methods, respectively.<br>
// <br>
// Because a PtTerminalConnection is associated with a PtConnection, it is
// therefore also associated with some PtCall. The PtTerminalConnection
// describes the specific relationship between a physical PtTerminal endpoint
// with respect to an address on a call. PtTerminalConnections provide a
// physical view of a call. For a particular PtAddress endpoint on a PtCall,
// there may be zero or more PtTerminals at which the call terminates. The
// PtTerminalConnection describes each specific PtTerminal on the call that
// is associated with a particular PtAddress endpoint on the call. Many
// simple applications may not care about which specific PtTerminals are on
// the PtCall at a particular PtAddress endpoint. In these cases, the logical
// view provided by PtConnections is sufficient.
//
// <H3>TerminalConnection States</H3>
// <p>
// The PtTerminalConnection has a state that describes the current
// relationship between a PtTerminal and a PtConnection.
// PtTerminalConnection states are distinct from PtConnection states.
// PtConnection states describe the relationship between an entire PtAddress
// endpoint and a PtCall, whereas the PtTerminalConnection state describes
// the relationship between one of the terminals at the endpoint address on
// the call with respect to its connection. Different terminals on a call
// which are associated with the same connection may be in different
// states.
// <p>
// The PtTerminalConnection class defines six states in the real world terms
// given below:<br><br>
// <dl>
// <dt>
// <b>PtTerminalConnection::IDLE</B></dt>
// <dd>
// The initial state for all PtTerminalConnections.
// PtTerminalConnection objects do not stay in this state for long.
// They typically transition into another state quickly.</dd>
// <dt>
// <b>PtTerminalConnection::RINGING</B></dt>
// <dd>
// Indicates the terminal is ringing, indicating that the terminal
// has an incoming call.</dd>
// <dt>
// <b>PtTerminalConnection::TALKING</B></dt>
// <dd>
// Indicates that the terminal is actively part of a PtCall,
// typically "off-hook", and that the party is communicating on the telephone
// call.</dd>
// <dt>
// <b>PtTerminalConnection::HELD</B></dt>
// <dd>
// Indicates that a terminal is part of a PtCall, but is on hold.
// Other PtTerminals which are on the same PtCall and associated with the same
// PtConnection may or may not also be in this state.</dd>
// <dt>
// <b>PtTerminalConnection::DROPPED</B></dt>
// <dd>
// Indicates that a particular PtTerminal has permanently left the
// telephone call.</dd>
// <dt>
// <b>PtTerminalConnection::UNKNOWN</B></dt>
// <dd>
// Indicates that the implementation is unable to determine the
// state of the PtTerminalConnection. PtTerminalConnections may transition
// into and out of this state at any time.</dd>
// </dl>
// <p>
// When a PtTerminalConnection moves into the PtTerminalConnection::DROPPED
// state, it is no longer associated with its PtConnection and PtTerminal.
// That is, both of these objects lose their references to the
// PtTerminalConnection. However, the PtTerminalConnection still maintains
// its references to the PtConnection and PtTerminal object for application
// reference. That is, when a PtTerminalConnection moves into the
// PtTerminalConnection::DROPPED state, the methods
// PtTerminalConnection.getConnection() and PtTerminalConnection.getTerminal()
// still return valid object pointers.
//
// <H3>Listeners and Events</H3>
// All events pertaining to the PtTerminalConnection object are reported via
// the PtCallListener objects for the Call object associated with this
// terminal connection. Events are reported to a PtCallListener when a new
// terminal connection is created and whenever a terminal connection changes
// state. Listeners are added to PtCall objects via the PtCall.addListener()
// method and more indirectly via the PtAddress.addCallListener() and
// PtTerminal.addCallListener() methods.<br>
// <br>
// All events pertaining to the PtTerminalConnection object are reported via
// the PtTerminalConnectionListener objects for the PtCall object associated
// with this terminal connection.  Events are reported to a
// PtTerminalConnectionListener when a new PtTerminalConnection is created
// and whenever a PtTerminalConnection changes state.  Listeners
// are added to PtCall objects via the PtCall.addCallListener() method and
// more indirectly via the PtAddress.addCallListener() and
// PtTerminal.addCallListener() methods.
class PtTerminalConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum TerminalConnectionState
   {
                IDLE    = 0x60,
                RINGING = 0x61,
                TALKING = 0x62,
                HELD    = 0x63,
                BRIDGED = 0x64,
                IN_USE  = 0x65,
                DROPPED = 0x66,
                UNKNOWN = 0x67
   };

/* ============================ CREATORS ================================== */

   PtTerminalConnection();
     //:Default constructor

        PtTerminalConnection(TaoClientTask *pClient,
                                                        const char* address,
                                                        const char* termName,
                                                        const char* callId,
                                                        int nIsLocal);

   PtTerminalConnection(const PtTerminalConnection& rPtTerminalConnection);
     //:Copy constructor (not implemented for this class)

   virtual
   ~PtTerminalConnection();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalConnection& operator=(const PtTerminalConnection& rhs);
     //:Assignment operator (not implemented for this class)

   virtual PtStatus answer();
     //:Answers an incoming telephone call on this PtTerminalConnection.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - Connection is in the PtConnection::DISCONNECTED state.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus hold();
     //:Places this PtTerminalConnection on hold with respect to the PtCall
     //:of which it is a part.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - This terminal connection is not in the PtTerminalConnection::TALKING state.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus unhold();
     //:Takes this PtTerminalConnection off hold with respect to the PtCall
     //:of which it is a part.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_STATE - This terminal connection is not in the PtTerminalConnection::HELD state.
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus startTone(int toneId, UtlBoolean local,
                      UtlBoolean remote, const char* locale=NULL);
     //:Starts playing the indicated tone
     // "toneId" is an integer that identifies which tone to play.
     // if "local" is TRUE, then play the tone for the near end listener.
     // if "remote" is TRUE, then play the tone for the far end listener.
     // The "locale" string is used for localization.  If it is NULL
     // play the tones that are appropriate for the U.S.A.
         // <br>
     // The current set of toneId's are assigned as follows:
     //    #define DTMF_TONES_BASE 512 (from .../include/mp/dtmflib.h)
     //!DTMF_0  = '0'
     //!DTMF_1  = '1'
     //!DTMF_2  = '2'
     //!DTMF_3  = '3'
     //!DTMF_4  = '4'
     //!DTMF_5  = '5'
     //!DTMF_6  = '6'
     //!DTMF_7  = '7'
     //!DTMF_8  = '8'
     //!DTMF_9  = '9'
     //!DTMF_*  = '*'
     //!DTMF_#  = '#'
     //!DTMF_TONE_DIALTONE   = (DTMF_TONES_BASE + 0)
     //!DTMF_TONE_BUSY       = (DTMF_TONES_BASE + 1)
     //!DTMF_TONE_RINGBACK   = (DTMF_TONES_BASE + 2)
     //!DTMF_TONE_RINGTONE   = (DTMF_TONES_BASE + 3)
     //!DTMF_TONE_CALLFAILED = (DTMF_TONES_BASE + 4)
     //!DTMF_TONE_SILENCE    = (DTMF_TONES_BASE + 5)


   virtual PtStatus stopTone(void);
     //:Stops playing the tone

   virtual PtStatus playFile(const char* audioFileName, UtlBoolean repeat,
                     UtlBoolean local, UtlBoolean remote);
     //:Play the audio file.  The name of the file to play is passed
     //:as an argument.
     //:"audioFileName" is the name of the audio file
     //!if "repeat" is TRUE, then play the audio file in a loop
     //!if "local" is TRUE, then play the audio for the near end listener
     //!if "remote" is TRUE, then play the audio for the far end listener

   virtual PtStatus playFile(FILE* audioFilePtr, UtlBoolean repeat,
                 UtlBoolean local, UtlBoolean remote);
     //:Play the audio file.  A pointer to the file to play is passed
     //:as an argument.
     //:"audioFilePtr" is a pointer to the open audio file
     //!if "repeat" is TRUE, then play the audio file in a loop
     //!if "local" is TRUE, then play the audio for the near end listener
     //!if "remote" is TRUE, then play the audio for the far end listener

   virtual PtStatus stopPlay(UtlBoolean closeFile);
     //:Stop playing the audio file
     //!if "closeFile is TRUE, then close the audio file.

   virtual PtStatus createPlayer(MpStreamPlayer** pPlayer, const char* szStream, int flags);
     //:Creates a Player

   virtual PtStatus destroyPlayer(MpStreamPlayer* pPlayer);
     //:Destroys a player

   virtual PtStatus createPlaylistPlayer(MpStreamPlaylistPlayer** pPlayer);
     //:Creates a playlist Player

   virtual PtStatus destroyPlaylistPlayer(MpStreamPlaylistPlayer* pPlayer);
     //:Destroys a playlist player

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getConnection(PtConnection& rConnection);
     //:Sets <i>rpConnection</i> to refer to the PtConnection corresponding
     //:to this terminal connection.
     //!param: (out) rpConnection - Reference to the PtConnection object corresponding to this terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getState(int& rState);
     //:Sets <i>rState</i> to the current state of the terminal connection,
     //:either DROPPED, HELD, IDLE, RINGING, TALKING or UNKNOWN.
     //!param: (out) rState - Set to the current state of the terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminal(PtTerminal& rTerminal);
     //:Sets <i>rpTerminal</i> to refer to the PtTerminal corresponding
     //:to this terminal connection.
     //!param: (out) rpTerminal - Reference to the PtTerminal object corresponding to this terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getTerminalName(UtlString& rTerminalName);
     //:Sets <i>rpTerminal</i> to refer to the PtTerminal corresponding
     //:to this terminal connection.
     //!param: (out) rTerminalName - Reference to the terminal name of this terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getAddressName(UtlString& rAddress);
     //:Sets <i>rpTerminal</i> to refer to the PtTerminal corresponding
     //:to this terminal connection.
     //!param: (out) rAddress - Reference to the adrress name of this terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getCallId(UtlString& rCallId);
     //:Sets <i>rpTerminal</i> to refer to the PtTerminal corresponding
     //:to this terminal connection.
     //!param: (out) rCallId - Reference to the call id of this terminal connection
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */
        virtual PtStatus isLocal(UtlBoolean& local);

friend class PtTerminalConnectionEvent;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PtTerminalConnection(const char* address, const char* terminalName, const char* callId, int isLocal);
         //:Protected constructor.

        OsTime          mTimeOut;

        void initialize();

        UtlString mAddress;
        UtlString mTerminalName;
        UtlString mCallId;
        char  mState;
        int   mIsLocal;

    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static int                               mRef;

        TaoClientTask   *mpClient;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalConnection_h_
