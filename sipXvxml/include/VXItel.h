/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************
 *
 * 
 *
 ************************************************************************
 */

#ifndef VXITEL_H
#define VXITEL_H

#include "VXItypes.h"                  /* For VXIchar definition */
#include "VXIvalue.h"

#include "VXIheaderPrefix.h"
#ifdef VXITEL_EXPORTS
#define VXITEL_API SYMBOL_EXPORT_DECL
#else
#define VXITEL_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name VXItel
 * @memo Telephony interface
 * @version 1.0
 * @doc 
 * VXItel provides the telephony functions for the VXI.  The transfer
 type is split into the bridge and blind transfers.  These platform
 functions are very platform and generally location dependant.
 */

/*@{*/

#define TEL_CONNECTTIMEOUT      L"vxi.tel.connecttimeout"    /* VXIInteger, in ms */
#define TEL_MAX_CALL_TIME       L"vxi.tel.maxcalltime"       /* VXIInteger, in ms */

#define TEL_TRANSFER_DURATION   L"vxi.tel.transfer.duration" /* VXIInteger, in ms */
#define TEL_TRANSFER_STATUS     L"vxi.tel.transfer.status"   /* VXIInteger, VXItelTransferStatus */
#define TEL_TRANSFER_AUDIO      L"vxi.tel.transfer.audio"

/**
 * Result codes for the telephony interface 
 *
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXItelResult {
  /** Fatal error, terminate call    */
  VXItel_RESULT_FATAL_ERROR        = -100,
  /** Low-level telephony library error */
  VXItel_RESULT_DRIVER_ERROR       =  -50,
  /** I/O error                      */
  VXItel_RESULT_IO_ERROR           =   -8,
  /** Out of memory                  */
  VXItel_RESULT_OUT_OF_MEMORY      =   -7,
  /** System error, out of service   */
  VXItel_RESULT_SYSTEM_ERROR       =   -6,
  /** Errors from platform services  */
  VXItel_RESULT_PLATFORM_ERROR     =   -5,
  /** Return buffer too small        */
  VXItel_RESULT_BUFFER_TOO_SMALL   =   -4,
  /** Property name is not valid    */
  VXItel_RESULT_INVALID_PROP_NAME  =   -3,
  /** Property value is not valid   */
  VXItel_RESULT_INVALID_PROP_VALUE =   -2,
  /** Invalid function argument      */
  VXItel_RESULT_INVALID_ARGUMENT   =   -1,
  /** Success                        */
  VXItel_RESULT_SUCCESS            =    0,
  /** Normal failure, nothing logged */
  VXItel_RESULT_FAILURE            =    1,
  /** Non-fatal non-specific error   */
  VXItel_RESULT_NON_FATAL_ERROR    =    2,
  /** Operation is not supported     */
  VXItel_RESULT_TIMEOUT            =    3,
  /** Operation is not supported     */
  VXItel_RESULT_UNSUPPORTED        =  100
} VXItelResult;


/**
  @name VXItelStatus
  @memo Telephony line status
  @doc A line status of Active indicates that the line is currently
  occupied. It may be in a call or in a transfer.
 */
typedef enum VXItelStatus
{
  /** In a call */
  VXItel_STATUS_ACTIVE,

  /** Not in call */
  VXItel_STATUS_INACTIVE

} VXItelStatus;


/**
  @name VXItelTransferStatus
  @memo Result codes for transfer requests
 */
typedef enum VXItelTransferStatus {
  /* The endpoint refused the call. */
  VXItel_TRANSFER_BUSY,

  /* There was no answer within the specified time. */
  VXItel_TRANSFER_NOANSWER,

  /* Some intermediate network refused the call. */
  VXItel_TRANSFER_NETWORK_BUSY,

  /* The call completed and was terminated by the caller. */
  VXItel_TRANSFER_NEAR_END_DISCONNECT,

  /* The call completed and was terminated by the callee. */
  VXItel_TRANSFER_FAR_END_DISCONNECT,

  /* The call completed and was terminated by the network. */
  VXItel_TRANSFER_NETWORK_DISCONNECT, 

  /* The call duration exceeded the value of maxtime attribute and was
     terminated by the platform. */
  VXItel_TRANSFER_MAXTIME_DISCONNECT,

  /* This value may be returned if the outcome of the transfer is unknown,
     for instance if the platform does not support reporting the outcome of
     blind transfer completion. */
  VXItel_TRANSFER_UNKNOWN

} VXItelTransferStatus;


/*
 ** Abstract interface for telephony functionality.
 **
 ** The telephony interface the handles VoiceBrowser telephony functionality:
 ** disconnect, blind transfer and supervised transfer.
 **
 ** There is one telephony interface per thread/line. 
 */

/*
** ==================================================
** VXItelInterface Interface definition
** ==================================================
*/

/** @name VXItelInterface
 ** @memo VXItel interface for telephony services
 **
 */
typedef struct VXItelInterface {
  /**
   * @name GetVersion
   * @memo Get the VXI interface version implemented
   *
   * @return  VXIint32 for the version number. The high high word is 
   *          the major version number, the low word is the minor version 
   *          number, using the native CPU/OS byte order. The current
   *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
   */ 
  VXIint32 (*GetVersion)(void);
  
  /**
   * @name GetImplementationName
   * @memo Get the name of the implementation
   *
   * @return  Implementation defined string that must be different from
   *          all other implementations. The recommended name is one
   *          where the interface name is prefixed by the implementator's
   *          Internet address in reverse order, such as com.xyz.rec for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   **/
  const VXIchar* (*GetImplementationName)(void);

  /**
   * @name BeginSession
   * @memo Reset for a new session.
   *
   * @doc
   * This must be called for each new session, allowing for
   * call specific handling to occur. For some implementations, this
   * can be a no-op.  For others runtime binding of resources or other 
   * call start specific handling may be done on this call. 
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for the new session
   *
   * @result VXItel_RESULT_SUCCESS on success 
   */
    VXItelResult (*BeginSession)(struct VXItelInterface  *pThis,  
                                 VXIMap                  *args);

  /**
   * @name EndSession
   * @memo Performs cleanup at the end of a call session.
   * @doc
   * This must be called at the termination of a call, allowing for
   * call specific termination to occur. For some implementations, 
   * this can be a no-op. For others runtime resources may be released
   * or other adaptation may be completed.
   *
   * @param args  [IN] Implementation defined input and output
   *                    arguments for ending the session
   *
   * @result VXItel_RESULT_SUCCESS on success
   */
  VXItelResult (*EndSession)(struct VXItelInterface  *pThis, 
                             VXIMap                  *args);

  /**
   * @name GetStatus
   * @memo Queries the status of the line.
   * @doc Returns information about the line during an execution.  
   * Use to determine if the line is up or down.
   * 
   * @param pThis [in] The line for which the status is to be queried.
   * @param status [out] A pointer to a pre-allocated holder for the status.
   *
   * @return VXItel_RESULT_SUCCESS if the status is queried successfully.
   *         VXItel_RESULT_INVALID_ARGUMENT if pThis is NULL or status is NULL.
   **/
  VXItelResult (*GetStatus)(struct VXItelInterface * pThis,
                            VXItelStatus *status);

  /**
   * @name Disconnect
   * @memo Immediately disconnects the caller on this line.
   *
   * @doc Disconnect the line. This sends the hardware into the 
   * out-of-service state where it will no longer generate events.
   * @param pThis [in] the VXItel object for which the call is to be rejected.
   *
   * @return VXItel_SUCCESS if operation succeeds,
   *         VXItel_INVALID_ARGUMENT if pThis is NULL,
   *         VXItel_HW_ERROR if an hardware error occurs,
   *         VXItel_NOT_INITIALIZED if VXItel was not properly initialized.
   *         VXItel_UNEXPECTED_RESULT if any other error occurs.
   **/
  VXItelResult (*Disconnect)(struct VXItelInterface *  pThis);

  /**
   * @name TransferBlind
   * @memo Performs a blind transfer.
   * @doc Perform a blind transfer into the network.  The implementation
   * is generally going to be platform dependant and will require a lot
   * of configuration per installation location.
   *
   * @param pThis the line on which the transfer is to be performed.
   *
   * @param properties [in] termination character, length, timeouts...
   *
   * @param transferDestination [in] identifier of transfer location (e.g. a
   *                                 phone number to dial or a SIP URL).
   *
   * @param data [in] the data to be sent to the transfer target.
   *
   * @param resp [out] Key/value containing the result.  Basic keys:
   *                   TEL_TRANSFER_DURATION - length of the call in ms
   *                   TEL_TRANSFER_STATUS - see VXItelTransferStatus
   *
   * @return VXItel_SUCCESS if operation succeeds,
   *         VXItel_INVALID_ARGUMENT if pThis or transferDestination is NULL,
   *         VXItel_HW_ERROR if an hardware error occurs,
   *         VXItel_NOT_INITIALIZED if VXItel was not properly initialized.
   *         VXItel_UNEXPECTED_RESULT if any other error occurs.
   **/
  VXItelResult (*TransferBlind)(struct VXItelInterface *  pThis,
				const VXIMap *properties,
                                const VXIchar *transferDestination,
                                const VXIMap *data,
				VXIMap **resp);

  /**
   * @name TransferBridge
   * @memo Performs a bridge transfer.
   *
   * @doc Perform a bridge transfer into the network.  The implementation
   * is generally going to be platform dependant and will require a lot
   * of configuration per installation location.
   *
   * @param pThis [in] the line on which the transfer is to be performed.
   *
   * @param properties [in] termination character, length, timeouts...
   *
   * @param transferDestination [in] identifier of transfer location (e.g. a
   *                                 phone number to dial or a SIP URL).
   *
   * @param data [in] the data to be sent to the transfer target.
   *
   * @param resp [out] Key/value containing the result.  Basic keys:
   *                   TEL_TRANSFER_DURATION - length of the call in ms
   *                   TEL_TRANSFER_STATUS - see VXItelTransferStatus
   *
   * @return VXItel_SUCCESS if operation succeeds,
   *         VXItel_INVALID_ARGUMENT if pThis is NULL or transferDestination is
   *                                 NULL,
   *         VXItel_HW_ERROR if an hardware error occurs,
   *         VXItel_NOT_INITIALIZED if VXItel was not properly initialized.
   *         VXItel_UNEXPECTED_RESULT if any other error occurs.
   **/
  VXItelResult (*TransferBridge)(struct VXItelInterface *  pThis,
				 const VXIMap *properties,
                                 const VXIchar* transferDestination,
				 const VXIMap *data,
				 VXIMap **resp);

} VXItelInterface;
/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */
