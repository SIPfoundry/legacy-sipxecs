//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef PLAYMESSAGESCGI_H
#define PLAYMESSAGESCGI_H

// SYSTEM INCLUDES
//#include <...>


// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Dynamically generates the VXML for
 * playing messages in the user's mailbox.
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class PlayMessagesCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    PlayMessagesCGI(const UtlBoolean& requestIsFromWebUI,
                    const UtlString& mailbox,
                                        const Url& from,
                                        const char* category,
                                        const char* blockSize,
                                        const char* nextBlockHandle,
                    const char* status,
                     const char* unheardMsgIdList,
                     const char* frameset);

    /**
     * Virtual Dtor
     */
    virtual ~PlayMessagesCGI();

    /** This does the work */
    virtual OsStatus execute (UtlString* out = NULL);

        /**
         *  Constructs the VXML script for playing the timestamp.
         *      @param timestamp        ACtual timestamp in the format timestamp is in the format "Mon, 8/26/2002 07:21:32 PM EST"
         *      @param promptUrl        Url of the prompts.
         *
         *      @return UtlString       VXML snippet for playing the timestamp.
         */
        UtlString getTimestampVxml(const UtlString& timestamp, const UtlString& promptUrl) const;

        OsStatus handleWebRequest( UtlString* out ) ;

        OsStatus handleOpenVXIRequest( UtlString* out ) ;

    /** Constructs the HTML for displaying the list of folders in a drop down box
        and the Move, Delete, Empty Folder buttons */
    OsStatus getFolderNamesSnippet( UtlString& rFolderSnippet ) const ;

    /** Constructs the HTML for displaying "More Messages : 1 2 3..." */
    OsStatus displayAdditionalMsgLinks( UtlString& rMsgLinksSnippet ) const ;

    /** Constructs the HTML for displaying the table listing the messages in the folder */
    OsStatus getHtmlContent( UtlString& rHtmlOutput,
                             const UtlString& cgiUrl) const;


protected:

private:

    /** Handles the display of voicemail inbox when user clicks on the link in the
     *  email notification, returns a frameset.
     */
     OsStatus handlePlayFromEmailNotification( UtlString* out );

    /* Returns the page title for the WebUI.
     * Page title is usually: "Contents of <foldername>"
     *
     * @param   rTitle    Filled with the page title on return.
     */
    OsStatus getPageTitle( UtlString& rTitle  ) const ;

    /** Converts the message duration to the desired format for
     *  display on the webUI.
     *
     *  @param unformattedDuration  Message duration in seconds.
     *  @param rFormattedDuration   Message duration in the format hh:mm:ss.
     *                              Filled on return.
     */
    OsStatus formatMessageDuration( const UtlString& unformattedDuration,
                                    UtlString& rFormattedDuration ) const ;


    /** Converts the message timestamp to the desired format for
     *  display on the webUI.
     *
     *  @param unformattedTimestamp Message timestamp in the format 12-Sep-2002 hh:mm:ss
     *  @param rFormattedTimestamp  Message timestamp in the format 12-Sep-02 hh:mm
     *                              Filled on return.
     */
    OsStatus formatTimestamp( const UtlString& unformattedTimestamp,
                              UtlString& rFormattedTimestamp ) const ;

    /** Converts the escaped from info into the desired format for
     *  display on the webUI.
     *
     */
    OsStatus formatFromInfo( const UtlString& unformattedFromInfo,
                             UtlString& rFormattedFromInfo,
                             const UtlBoolean& firstentry) const ;


        /**
         *      Validates the URL of the caller who left voicemail to determine if:
         *      1/ the caller was an internal SIPxchange user or an outside caller
         *      2/ if a sipxchange user, to determine if he has voicemail permission.
         *
         *      @param  fromStr                         Identity of the caller
         *      @param  rVoicemailEnabled       Boolean to indicate that the caller has
         *                                                              voicemail inbox. Filled on return.
         *      @param  rFromExtension          Extension of the caller. Filled on return.
         *      @param  rFromMailboxIdentity Mailbox id of the caller. Filled on return.
         *
         *      @return OsStatus        OS_SUCCESS      if the caller is an internal SIPxchange user
         *                                              OS_FAILED       if outside caller
         */
        OsStatus validateFromUrl(       const UtlString& fromStr,
                                                                UtlBoolean& rVoicemailEnabled,
                                                                UtlString& rFromExtension,
                                                                UtlString& rFromMailboxIdentity) const ;

        /** Constructs the VXML snippet for playing message origin information.
         *
         *      @param  callerIsSipxUser        Indicates if the caller was an internal SIPxchange user
         *      @param  voicemailEnabled        Boolean to indicate if the caller has voicemail inbox
         *      @param  fromExtension           Extension of the caller
         *      @param  fromMailboxIdentity Identity of the caller
         *      @param  unformattedFrom         Full identity of the caller, including display name
         *      @param  promptUrl                       Base URL for accessing prompts
         *
         *      @return UtlString                       VXML snippet for playing message origin information
         */
        UtlString getMsgOriginVxml ( const OsStatus& callerIsSipxUser,
                                                                const UtlBoolean& voicemailEnabled,
                                                                const UtlString& fromExtension,
                                                                const UtlString& fromMailboxIdentity,
                                                                const UtlString& unformattedFrom,
                                                                const UtlString& promptUrl) const ;

        /** Constructs the VXML snippet for playing the message menu.
         *      The options of the message menu played after the message varies depending
         *      on the type of messages being heard (inbox, saved, deleted), the
         *      status of the caller (internal SIPxchange user with voicemail, outside caller).
         *
         *      @param  promptUrl                       Base URL for accessing prompts
         *      @param  callerIsSipxUser        Indicates if the caller was an internal SIPxchange user
         *      @param  voicemailEnabled        Boolean to indicate if the caller has voicemail inbox
         *
         *      @return UtlString                       VXML snippet for playing the message menu
         */
        UtlString getMessageMenuVxml (  const UtlString& promptUrl,
                                                                        const OsStatus& callerIsSipxUser,
                                                                        const UtlBoolean& voicemailEnabled) const ;



        /** Fully qualified mailbox id. For example, sip:hari@pingtel.com */
    UtlString m_mailboxIdentity;

        /** Fully qualified id of the user who called in. */
        const Url& m_from;

        /** Category of the messages retrieved. It can take the following values:
         *      1. unheard -- unheard messages in the inbox.
         *      2. heard   -- heard messages in the inbox.
         *  3. inbox   -- all messages in the inbox folder. (for WebUI use)
         *      3. saved   -- all the messages in the saved folder.
         *      4. custom folder name -- all messages in that custom folder.
         */
        UtlString m_category;

        /** Number of messages retrieved by this CGI.
         *  This is set to a reasonable value like 10 so that there is less
         *  thrashing on the file system.
     *
     *  In the case of the WebUI, this is set to the value specified in the
     *  voicemail.xml file.
         */
        int m_iBlockSize;

        /**     Id of the last message in the previous block.
         *      This acts a cursor to track where we left off.
         *      Defaults to -1 when the messages are retrieved for the first time.
         */
        int m_iNextBlockHandle;
    UtlString m_strNextBlockHandle ;

    /** Flag indicating if this CGI was called from the User UI */
    const UtlBoolean m_fromWeb ;

    /** Status code */
    const UtlString m_status ;

    /** Id of the last unheard message that was played. */
    const UtlString m_unheardMsgIdList;

     /** For a request from the Web UI, we should generate a frame set for the
      *  voicemail inbox page.  That request will have a 'frameset' parameter
      *  in the url
      */
     const UtlString m_frameSet;

};

#endif //PLAYMESSAGESCGI_H
