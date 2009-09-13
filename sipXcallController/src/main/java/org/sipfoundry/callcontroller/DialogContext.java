/*
 * 
 * 
 * Copyright (C) 2009 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.DialogExt;

import java.util.HashSet;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.SipProvider;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.RestServer;

/**
 * One of these associated with a dialog. Stores the current call status. (sent to us via a NOTIFY ).
 */
public class DialogContext {

    private static Logger logger = Logger.getLogger(DialogContext.class);
    private String key;
    private StringBuffer status = new StringBuffer();
    private HashSet<Dialog> dialogs = new HashSet<Dialog>();

    public DialogContext(String key, int timeout) {
        this.key = key;
        logger.debug("DialogContext : " + timeout);
        status.append("<status-lines>");

        RestServer.timer.schedule(new TimerTask() {

            @Override
            public void run() {
                for (Dialog dialog : dialogs) {
                    try {
                        if (dialog.getState() != DialogState.CONFIRMED
                                && dialog.getState() != DialogState.TERMINATED) {
                            Request byeRequest = dialog.createRequest(Request.BYE);
                            SipProvider provider = ((DialogExt) dialog).getSipProvider();
                            ClientTransaction ctx = provider.getNewClientTransaction(byeRequest);
                            dialog.sendRequest(ctx);
                        } else {
                            logger.debug("Not firing BYE message " + dialog.getState());
                        }
                    } catch (Exception ex) {
                        logger.error("Exception caught", ex);
                    }
                }
            }

        }, timeout * 1000);
    }

    public void remove() {
        RestServer.timer.schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    if ( dialogs.isEmpty () ) {
                        SipUtils.removeDialogContext(key);
                    }
                } catch (Exception ex) {
                    logger.error("Exception caught creating instance", ex);
                }
            }
        }, 30 * 1000);

    }

    public void setStatus(String status) {
        this.status.append("\n<status-line>"+status+"</status-line>");
    }

    /**
     * @return the status
     */
    public String getStatus() {
        return status.toString() + "\n</status-lines>";
    }

    public void addDialog(Dialog dialog) {
        logger.debug("addDialog " + this.dialogs);
        this.dialogs.add(dialog);

    }

}
