//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipDialogMgr_h_
#define _SipDialogMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <utl/UtlString.h>
#include <utl/UtlHashBag.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class SipDialog;

// TYPEDEFS

//! Class for maintaining information about SIP dialogs.
class SipDialogMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   //! Describes the CSeq of a request relative to the current CSeq in the dialog.
   enum transactionSequence
   {
      NO_DIALOG = 0,            ///< No matching dialog found.
      OUT_OF_ORDER,             /**< CSeq < last CSeq seen
                                 *   Should receive 500 response. */
      IN_ORDER,                 ///< CSeq > last CSeq seen
      LOOPED                    /**< CSeq has been seen, but branch
                                 *   param. differs  Should receive 482
                                 *   response. */
   };

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    SipDialogMgr();


    //! Destructor
    virtual
    ~SipDialogMgr();


/* ============================ MANIPULATORS ============================== */

    //! Create a new dialog for the given SIP message
    UtlBoolean createDialog(const SipMessage& message,
                            UtlBoolean messageIsFromLocalSide,
                            const char* dialogHandle = NULL);

    //! Update the dialog information for the given message
    /*! If a dialog matches this message update the dialog information
     *  otherwise if this message is part of an established dialog and
     *  matches an early dialog change the dialog to established and
     *  update the dialog information.
     */
    UtlBoolean updateDialog(const SipMessage& message,
                            const char* dialogHandle = NULL);

    //! Delete the dialog for the given dialog handle
    UtlBoolean deleteDialog(const char* dialogHandle);


    //! Get the dialog-related fields and set them in the given request
    /*! Increments the dialog's local Cseq as well.
     */
    UtlBoolean setNextLocalTransactionInfo(SipMessage& request,
                                           const char* method = NULL,
                                           const char* dialogHandle = NULL);

    //! Set the next local CSeq for a dialog.
    /** Finds the dialog with the specified handle and sets its stored
     *  "next local CSeq" value to the specified number.
     *  Returns TRUE if the handle was found, and FALSE if not.
     *  Not recommended for use on ongoing dialogs, but is useful
     *  for importing the state of an existing dialog.
     */
    UtlBoolean setNextLocalCseq(const UtlString& dialogHandleString,
                                int nextLocalCseq);

    /* ============================ ACCESSORS ================================= */

    //! Get the early dialog handle for the given established dialog handle
    /*! This works even if the SipDialog is an early dialog that has not yet
     *  been updated to be an established dialog. */
    UtlBoolean getEarlyDialogHandleFor(const char* establishedDialogHandle,
                                       UtlString& earlyDialogHandle);

    //! Get the established dialog for the given early dialog
    UtlBoolean getEstablishedDialogHandleFor(const char* earlyDialogHandle,
                                             UtlString& establishedDialogHandle);

    //! Get a count of the SipDialogs
    int countDialogs() const;

    //! Get dump string of dialogs
    int toString(UtlString& dumpString);

/* ============================ INQUIRY =================================== */

    //! Is there an early dialog that matches this early dialogHandle
    /*! If earlyDialogHandle is not an early dialog, no matches are
     * considered to exist.
     */
    UtlBoolean earlyDialogExists(const char* earlyDialogHandle);

    //! Is there an early dialog that matches this established dialogHandle
    /*! If establishedDialogHandle is not an established dialog, no matches are
     * considered to exist.
     */
    UtlBoolean earlyDialogExistsFor(const char* establishedDialogHandle);

    //! Is there a dialog that matches this dialogHandle
    /*! If the dialog handle is an early dialog, it will only match
     *  early dialogs.  If the dialog handle is an established dialog
     *  it will only match established dialogs.
     */
    UtlBoolean dialogExists(const char* dialogHandle);

    //! Checks to see if the given message matches the last local transaction
    UtlBoolean isLastLocalTransaction(const SipMessage& message,
                                      const char* dialogHandle = NULL);

    /** Check if the message is part of a known dialog, and if so,
     *  how it fits in the sequence of requests from the far end of
     *  the dialog.
     *  Note:  This function assumes that the message is not an exact
     *  duplicate (same CSeq and branch parameter) of a previous request,
     *  since those are usually absorbed by an earlier stage of processing.
     *  Hence, any message with the current CSeq is assumed to have a
     *  different branch parameter, and thus be a looped request (possibly
     *  due to a transport error).
     */
    enum transactionSequence isNewRemoteTransaction(const SipMessage& sipMessage);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    SipDialogMgr(const SipDialogMgr& rSipDialogMgr);

    //! Assignment operator NOT ALLOWED
    SipDialogMgr& operator=(const SipDialogMgr& rhs);

    //! Find a dialog that matches, optionally look for an early dialog if exact match does not exist
    /*! Checks tags in both directions
     */
    SipDialog* findDialog(const UtlString& dialogHandle,
                          UtlBoolean ifHandleEstablishedFindEarlyDialog,
                          UtlBoolean ifHandleEarlyFindEstablishedDialog);

    //! Find a dialog that matches, optionally look for an early dialog if exact match does not exist
    /*! Checks tags in both directions
     */
    SipDialog* findDialog(UtlString& callId,
                          UtlString& localTag,
                          UtlString& remoteTag,
                          UtlBoolean ifHandleEstablishedFindEarlyDialog,
                          UtlBoolean ifHandleEarlyFindEstablishedDialog);

    //! lock for single thread use
    void lock();

    //! unlock for use
    void unlock();

    OsMutex mDialogMgrMutex;
    UtlHashBag mDialogs;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipDialogMgr_h_
