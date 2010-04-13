//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsStunDatagramSocket_h_
#define _OsStunDatagramSocket_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDatagramSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"

// DEFINES
#define STUN_TIMEOUT_RESPONSE_MS                500  /**< Wait at most 500ms for a stun response    */
#define STUN_INITIAL_REFRESH_REPORT_THRESHOLD   5    /**< First complain after failing N times */
#define STUN_REFRESH_REPORT_THRESHOLD           120  /**< After success, complain after N times */
#define STUN_ABORT_THRESHOLD                    60   /**< Abort after failing to keep-alive N times
                                                          if we never succeeded. */
#define STUN_FAILURE_REFRESH_PERIOD_SECS        1    /**< How often to retry under error */

#define STUN_MSG_TYPE        (OsMsg::USER_START + 1) /**< Stun Msg type/Id */

#define STUN_OPTION_NORMAL                      0   /**< Do not change port and address */
#define STUN_OPTION_CHANGE_PORT                 1   /**< Request responses send from a different port */
#define STUN_OPTION_CHANGE_ADDRESS              2   /**< Request responses send from a different IP */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class StunMsg ;
class OsEvent ;
class OsStunAgentTask;

/**
 * OsStunDatagramSocket extends an OsDatagramSocket by adding an integrated
 * STUN client.  If STUN is enabled, STUN request will be send to the
 * designated server every refresh period.  The external address obtained by
 * stun is retrieved by invoking getExternalIp.
 *
 * For this mechanism to work, someone must pump inbound socket data by
 * calling one of the ::read() methods.  Otherwise, the stun packets will
 * not be received/processed.  Internally, the implemenation peeks at the
 * read data and passes the stun message to the OsStunQueryAgent for
 * processing.
 *
 *
 */
class OsStunDatagramSocket : public OsDatagramSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsStunAgentTask ;

/* ============================ CREATORS ================================== */
    /**
     * Constructor accepting the remote host port, name, localhost port,
     * name, and stun attributes.
     *
     * @param remoteHostPort Port of the remote host for connection-like use of
     *        OsDatagramSocket.  Alternatively, you can also use the ::WriteTo
     *        method and include the host/port at sent time.
     * @param remoteHostName Hostname of remote host for a connection-like use
     *        of OsDatagramSocket.
     * @param localHostPort Local port number for the socket,
     *        PORT_DEFAULT to autoselect
     * @param localHostName Local host name for the socket (e.g. which interface
     *        to bind on.
     * @param bEnableStun Enable stun for this socket instance.
     * @param szStunServer Default stun server
     * @param iRefreshPeriodInSec How often to refresh the stun binding
     *        (keep alive).
     * @param iStunOptions Optional bitwise-or of STUN_CHANGE_* symbols.
     * @param pNotification Optional notification event that is signaled upon
     *        the initial successful stun response or on failure (did not
     *        receive a stun response within (STUN_ABORT_THRESHOLD *
     *        STUN_TIMEOUT_RESPONSE_MS).
     */
    OsStunDatagramSocket(int remoteHostPort,
                         const char* remoteHostName,
                         int localHostPort = PORT_DEFAULT,
                         const char* localHostName = NULL,
                         bool bEnableStun = FALSE,
                         const char* szStunServer = NULL,
                         int iRefreshPeriodInSec = 0,
                         int iStunOptions = STUN_OPTION_NORMAL,
                         OsNotification* pNotification = NULL) ;

    /**
     * Standard Destructor
     */
    virtual ~OsStunDatagramSocket();

/* ============================ MANIPULATORS ============================== */

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength) ;

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            UtlString* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength, long waitMilliseconds);

    /**
     * Set the STUN keep alive period in seconds.  If a keep alive period was
     * already set and STUN was enabled this method will abort the current
     * timer and set a new timer from now.  If is safest to manually call
     * refreshStunBinding(FALSE) immediate after resetting the keep alive
     * period.
     *
     * @param secs New keep alive period in seconds.  Must be >= 0.  A value
     *        of 0 (zero) disables the keep alive mechanism.
     */
    virtual void setKeepAlivePeriod(int secs) ;

    /**
     * Set the STUN server.  The new stun server will be used on the next
     * timer refresh or manual call to refreshStunPacket.
     *
     * @param szHostname Hostname or IP address for the stun server.  The
     *        default STUN port (3478) will be used.
     */
    virtual void setStunServer(const char* szHostname) ;

    /**
     * Set the STUN client options.
     *
     * @param stunOptions See the STUN_ defines for possible options.
     */
    virtual void setStunOptions(int stunOptions) ;

    /**
     * Enable or disable STUN.  Disabling STUN will stop all keep alives
     * and cause getExternalIp to fail.  Enabling STUN will reset the the
     * keep alive timer and will force a binding refresh.
     *
     * @param bEnable True to enable STUN or false to disable.
     */
    virtual void enableStun(bool bEnable) ;

    /**
     * When a stun packet is received this socket can either call read again
     * to obtain the next packet or return with zero bytes read.  By default
     * the socket will transparently call Read again and will block until a
     * non-stun packet is read. Calling this method will effect the next
     * read -- in other words it will not unblock an active read.
     *
     * @param bEnable True to enable transparent stun reads and block until
     *        a non-stun packet is received (default) or False to return
     *        with zero bytes read if a stun packet is received.
     */
    virtual void enableTransparentStunReads(bool bEnable) ;

    /**
     * Refresh the stun binding by sending a stun request and looking at the
     * the results.  This method will block for upto STUN_TIMEOUT_RESPONSE_MS
     * milliseconds for a response.
     *
     * @param bFromReadSocket True to read immediately from the socket or False
     *        to assume that someone is already reading from the socket.
     *        In most cases someone else is reading from the socket, and you
     *        should specify false.
     */
    virtual void refreshStunBinding(UtlBoolean bFromReadSocket = false) ;

    /**
     * Read a STUN packet from the socket.  All non-STUN traffic will be discarded.
     *
     * @param buffer Buffer to place stun packet
     * @param bufferLength Lenth of buffer in bytes
     * @param rTimeout How long to wait for a stun packet/response.   The
     *        STUN_TIMEOUT_RESPONSE_MS timeout is recommended.
     */
    virtual int readStunPacket(char* buffer, int bufferLength, const OsTime& rTimeout) ;

    /**
     * Add an alternate destination to this OsStunDatagramSocket.  Alternate
     * destinations are tested by sending stun packets.  If a stun response is
     * received and the priority is greater than what has already been selected
     * then that address is used.
     *
     * @param szAddress IP address of the alternate destination
     * @param iPort port number of the alternate destination
     * @param cPriority priority of the alternate where a higher number
     *        indicates a higher priority.
     */
    virtual void addAlternateDestination(const char* szAddress, int iPort, unsigned char cPriority) ;

    /**
     * Sets as notification event that is signaled upon the next successful
     * stun response or on failure (did not receive a stun response within
     * (STUN_ABORT_THRESHOLD * STUN_TIMEOUT_RESPONSE_MS).  If a notification
     * event was previous set either by calling this method or via the
     * constructor, it will be overridden.  If the initial STUN success/failure
     * state has already been determined, this method is undefined.
     *
     * @param pNotification Notification event signaled on success or failure.
     */
    virtual void setNotifier(OsNotification* pNotification) ;

/* ============================ ACCESSORS ================================= */

    /**
     * Return the external IP address for this socket.  This method will return
     * false if stun is disabled, it was unable to retrieve a stun binding, or
     * both the ip and port parameters are null.
     *
     * @param ip Buffer to place STUN-discovered IP address
     * @param port Buffer to place STUN-discovered port number
     */
    virtual UtlBoolean getExternalIp(UtlString* ip, int* port) ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /**
     * Set the STUN-derived address for this OsStunDatagramSocket
     *
     * @param address STUN-derived hostname/IP address
     * @param iPort STUN-derived port address
     */
    void setStunAddress(const UtlString& address, const int iPort) ;

    /**
     * Report that a stun attempt failed.
     */
    void markStunFailure() ;


    /**
     * Report that a stun attempt succeeded.
     */
    void markStunSuccess() ;


    /**
     * Reset the destination address for this OsStunDatagramSocket.  This
     * method is called by the OsStunAgentTask when a better address is
     * found via STUN/ICE.
     *
     * @param address The new destination address
     * @param iPort The new destination port
     * @param cPriority Priority of the destination address
     */
    void setDestinationAddress(const UtlString& address, int iPort, unsigned char cPriority) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mKeepAlivePeriod ;      /**< Keep alive/refresh period */
    int mCurrentKeepAlivePeriod;/**< Current keep alive period -- may be
                                     accelerated under error conditions */
    UtlString mStunServer ;     /**< stun server name */
    int mStunOptions ;          /**< STUN client options */
    int mStunPort ;             /**< port reported by stun process */
    UtlString mStunAddress ;    /**< ip address reported by stun process */
    OsTimer* mpTimer ;          /**< timer used for keep alives */
    OsQueuedEvent* mpRefreshEvent ; /**< Refresh event (performs keep alive) */
    bool mbEnabled ;            /**< Is stun enabled? */
    int mStunRefreshErrors ;    /**< Number of consecutive STUN refresh errors */
    OsStunAgentTask* pStunAgent;/**< Reference to the stun agent task */
    bool mbTransparentStunRead ;/**< Block until a non-stun packet is read */
    UtlString mDestAddress;     /**< Destination address */
    int miDestPort ;            /**< Destination port */
    unsigned char mcDestPriority ; /**< Priority of destination address / port. */
    OsNotification* mpNotification ; /** Notify on initial success or failure */

};

/* ============================ INLINE METHODS ============================ */


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

//: Basic StunMsg -- relies on external bodies to allocate and free memory.
class StunMsg : public OsMsg
{
public:
/* ============================ CREATORS ================================== */
   StunMsg(char*                 szBuffer,
           int                   nLength,
           OsStunDatagramSocket* pSocket,
           UtlString             receivedIp,
           int                   iReceivedPort);
     //:Constructor

   StunMsg(const StunMsg& rStunMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~StunMsg();
     //:Destructor
/* ============================ MANIPULATORS ============================== */

   StunMsg& operator=(const StunMsg& rhs);
     //:Assignment operator
/* ============================ ACCESSORS ================================= */

   char* getBuffer() const ;

   int getLength() const ;

   OsStunDatagramSocket* getSocket() const ;

   UtlString getReceivedIp() const ;

   int getReceivedPort() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    char*                 mBuffer ;
    int                   mLength ;
    OsStunDatagramSocket* mpSocket ;
    UtlString             mReceivedIp ;
    int                   miReceivedPort ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
};


#endif  // _OsStunDatagramSocket_h_
