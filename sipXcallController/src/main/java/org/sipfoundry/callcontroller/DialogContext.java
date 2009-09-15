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
    public static String HEADER = "<?xml version=\"1.0\" ?>" + "\n<status-lines "
            + "xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/call-status-00-00\">";
    public static String FOOTER = "\n</status-lines>\n";
    
    private int timeout;
    private long creationTime = System.currentTimeMillis();

    public DialogContext(String key, int timeout) {
        this.key = key;
        logger.debug("DialogContext : " + timeout);
        status.append(HEADER);
        this.timeout = timeout;
       
        RestServer.timer.schedule(new TimerTask() {

            @Override
            public void run() {
                for (Dialog dialog : dialogs) {
                    try {
                        if (dialog.getState() != DialogState.CONFIRMED
                                && dialog.getState() != DialogState.TERMINATED) {
                            SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                        } else {
                            logger.debug("Not firing BYE message " + dialog.getState());
                        }
                    } catch (Exception ex) {
                        logger.error("Exception caught", ex);
                    }
                }
            }

        }, (timeout + 1) * 1000);
    }

    public void remove() {
        try {
            for (Dialog dialog : dialogs) {
                if (dialog.getState() != DialogState.TERMINATED) {
                    SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                } else {
                    this.removeMe(dialog);
                }

            }
        } catch (Exception ex) {
            logger.error("Exception caught", ex);
        }
    }

    public void setStatus(String callId, String method, String status) {
        this.status.append("\n<status>");
        this.status.append("\n\t<timestamp>" + System.currentTimeMillis() + "</timestamp>");
        this.status.append("\n\t<call-id>" + callId + "</call-id>");
        this.status.append("\n\t<method>" + method + "</method>");
        this.status.append("\n\t<status-line>" + status.trim() + "</status-line>");
        this.status.append("\n</status>");
    }

    /**
     * @return the status
     */
    public String getStatus() {
        return status.toString() + FOOTER;
    }

    public void addDialog(Dialog dialog) {
        logger.debug("addDialog " + this.dialogs);
        this.dialogs.add(dialog);

    }

    public void removeMe(Dialog dialog) {
        logger.debug("removeMe " + dialog);
        this.dialogs.remove(dialog);
        if (dialogs.isEmpty()) {
            if (  System.currentTimeMillis() - this.creationTime >= timeout*1000 ) {
                SipUtils.removeDialogContext(key, this);
            } else {
                long delta = timeout*1000 - (System.currentTimeMillis() - this.creationTime) ;
                RestServer.timer.schedule( new TimerTask() {

                    @Override
                    public void run() {
                        SipUtils.removeDialogContext(key, DialogContext.this);
                    }
                    
                }, delta );
            }
        }
    }

}
