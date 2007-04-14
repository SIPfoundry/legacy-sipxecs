// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <malloc.h>

// APPLICATION INCLUDES
#include "os/OsConnectionSocket.h"
#include "processcgi/processXMLCommon.h"
#include "os/OsProcessMgr.h"
#include "os/OsSysLog.h"
#include "os/OsTokenizer.h"
#include "EmailReporter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
EmailReporter::EmailReporter() :
mNumContacts(0),
mNumMessages(0)
{
}

// Copy constructor
EmailReporter::EmailReporter(const EmailReporter& rEmailReporter)
{
}

// Destructor
EmailReporter::~EmailReporter()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
EmailReporter&
EmailReporter::operator=(const EmailReporter& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

OsStatus EmailReporter::report(UtlString &rProcessAlias,UtlString &rMessage)
{
    OsStatus retval = OS_FAILED;

    if (mNumMessages < MAX_MESSAGES)
    {
        mFailureMessage[mNumMessages] = new UtlString(rProcessAlias);
        mFailureMessage[mNumMessages]->append(":");
        mFailureMessage[mNumMessages]->append(rMessage);;
        mNumMessages++;

        retval = OS_SUCCESS;
    }
    else
    {
        char msg[256];
        sprintf(msg,"ERROR: Max number of messages reached for %s\n",rProcessAlias.data());
        OsSysLog::add(FAC_WATCHDOG,PRI_ERR,msg);
#ifdef DEBUG
        osPrintf(msg);
#endif /* DEBUG */
    }

    return retval;

}

OsStatus EmailReporter::send()
{
    OsStatus retval = OS_FAILED;
    OsProcessMgr *processMgr = OsProcessMgr::getInstance(SIPX_TMPDIR);
    int numSent = 0;

    OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Checking if reports need to be e-mailed");

    //execute the mail command for each user
    for (int loop = 0; loop < mNumContacts;loop++)
    {
        UtlString CommandWithUserStr = mEmailCommandStr;
        //now replace the %CONTACT% with the actual e-mail address
        //first find out where %CONTACT% is...
        unsigned int body_pos = CommandWithUserStr.index("%BODY%");
        unsigned int pos = CommandWithUserStr.index("%CONTACT%");
        if ((pos != UTL_NOT_FOUND) && (body_pos != UTL_NOT_FOUND))
        {
            //and now let's execute it!
            if (mNumMessages)
            {
                OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Sending mail to: %s",mContacts[loop]->data());
                UtlString body;
                UtlString sHostname;
                OsSocket::getHostName(&sHostname) ;
                body = "Message from SIPxchange server: ";
                body.append(sHostname);
                body.append("\n");

                for (int loop2 = 0;loop2 < mNumMessages;loop2++)
                {

                    body.append(mFailureMessage[loop2]->data());
                    body.append("\n");
                }

                if (mNumMessages)
                {
                    //ok, now we found it. lets replace %CONTACT% with the user.
                    CommandWithUserStr = CommandWithUserStr.replace(pos,9,mContacts[loop]->data());

                    //recalculate pos of body because we changed the size
                    int body_pos = CommandWithUserStr.index("%BODY%");

                    //now replace body
                    CommandWithUserStr = CommandWithUserStr.replace(body_pos,6,body.data());

                    //copy to our workbuf before we parse
                    char *workbuf = strdup(CommandWithUserStr.data());
                    int numargs = 0;
                    pt_token_t *toklist = parse_tokenize(workbuf, &numargs);

                    //now allocate space for arg list
                    UtlString *args = new UtlString[numargs+1];

                    for (int loop3 = 0;loop3 < numargs;loop3++)
                    {
                        args[loop3] = parse_token(toklist, loop3);
                    }
                    parse_kill(toklist);
                    free(workbuf);

                    UtlString mailAlias = "WatchDogMail_";
                    mailAlias.append(mContacts[loop]->data());

                    UtlString startupDir = ".";

                    processMgr->startProcess(mailAlias,args[0],&args[1],startupDir);
                    delete [] args;

                    OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Sent message to %s",mContacts[loop]->data());
                    numSent++;
                }

                OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Sending DONE.");
            }


        }
        else
        {
            const char *msg = "ERROR: %CONTACT% and %BODY% must be in the email execute tag in the xml file.\n";
            OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"%s",msg);
#ifdef DEBUG
            osPrintf("%s",msg);
#endif /* DEBUG */
        }
    }

    OsSysLog::add(FAC_WATCHDOG,PRI_INFO,"Done EMAIL reports. %d sent",numSent);

    flush();

    return retval;
}


//clears out the queue
void EmailReporter::flush()
{
    //now clean up the message queue
    for (int loop2 = 0;loop2 < mNumMessages;loop2++)
    {
        if (mFailureMessage[loop2])
            delete mFailureMessage[loop2];
    }

    mNumMessages = 0;
}

void EmailReporter::setEmailExecuteCommand(UtlString &rCommand)
{
    mEmailCommandStr = rCommand;
}

OsStatus EmailReporter::addContact(UtlString &rEmailAddress)
{
    OsStatus retval = OS_FAILED;

    if (mNumContacts < MAX_CONTACTS)
    {
        mContacts[mNumContacts] = new UtlString(rEmailAddress);
        mNumContacts++;
        retval = OS_SUCCESS;
    }
    else
    {
        const char *msg = "ERROR: Max number of contacts reached!\n";
        OsSysLog::add(FAC_WATCHDOG,PRI_ERR,"%s",msg);
#ifdef DEBUG
        osPrintf(msg);
#endif /* DEBUG */
    }

    return retval;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

