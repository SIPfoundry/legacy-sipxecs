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
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import java.util.HashMap;
import java.util.HashSet;
import java.util.TimerTask;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.SipException;
import javax.sip.SipProvider;
import javax.sip.header.ReasonHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipStackBean;

/**
 * One of these associated with a dialog. Stores the current call status. (sent to us via a NOTIFY ).
 */
public class DialogContext {

    private Response lastResponse;
    private SipStackBean stackBean;
    private static Logger logger = Logger.getLogger(DialogContext.class);
    private String key;
    private StringBuffer status = new StringBuffer();
    private HashSet<Dialog> dialogs = new HashSet<Dialog>();
    private HashMap<Dialog,Request> dialogTable = new HashMap<Dialog,Request>();
    public static String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "\n<status-lines "
            + "xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/call-status-00-00\">";
    public static String FOOTER = "\n</status-lines>\n";

    private int cacheTimeout;
    private long creationTime = System.currentTimeMillis();
    private UserCredentialHash userCredentials;

    public DialogContext(String key, int timeout, int cachetimeout) {
        this.key = key;
        logger.debug("DialogContext : " + timeout + " cachetimeout " + cachetimeout);
        status.append(HEADER);
        this.cacheTimeout = cachetimeout;

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
            HashSet<Dialog> removeSet = new HashSet<Dialog>();
            for (Dialog dialog : dialogs) {
                if (dialog.getState() != DialogState.TERMINATED) {
                    SipListenerImpl.getInstance().getHelper().tearDownDialog(dialog);
                } else {
                    removeSet.add(dialog);
                }

            }
            for (Dialog dialog: removeSet) {
                this.removeMe(dialog);
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

    public void addDialog(Dialog dialog, Request request) {
        logger.debug("addDialog " + this.dialogs);
        this.dialogs.add(dialog);
        dialog.setApplicationData(this);
        this.dialogTable.put(dialog,request);
    }

    public Dialog getPeer(Dialog dialog) {
        Object[] dialogs = this.dialogs.toArray();
        assert (dialogs[0] == dialog || dialogs[1] == dialog);
        return dialogs[0] == dialog ? (Dialog) dialogs[1] : (Dialog) dialogs[0];
    }

    public void setLastResponse(Response lastResponse) {
        this.lastResponse = lastResponse;
    }

    public Response getLastResponse() {
        return lastResponse;
    }

    public void tearDownDialogs(String reason) {
        logger.debug("Tearinging Down Dialogs");
        for (Dialog dialog : dialogs) {
            try {
                if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                    Request request = dialog.createRequest(Request.BYE);
                    ReasonHeader reasonHeader = SipListenerImpl.getInstance().getHelper()
                            .createReasonHeader(reason);
                    request.addHeader(reasonHeader);
                    SipProvider provider = ((DialogExt) dialog).getSipProvider();
                    ClientTransaction ctx = provider.getNewClientTransaction(request);
                    dialog.sendRequest(ctx);
                } else if (dialog != null && dialog.getState() != DialogState.TERMINATED) {
                    dialog.delete();
                }
            } catch (Exception e) {
                logger.error("Unexpected exception sending BYE", e);
            }
        }
    }

    public void tearDownDialogs() {
        this.tearDownDialogs(null);
    }

    public void removeMe(Dialog dialog) {
        logger.debug("removeMe " + dialog);
        logger.debug("removeMe: dialogs = " + this.dialogs);
        this.dialogs.remove(dialog);
        this.dialogTable.remove(dialog);
        if (dialogs.isEmpty()) {
            if (System.currentTimeMillis() - this.creationTime >= cacheTimeout * 1000) {
                SipUtils.removeDialogContext(key, this);
            } else {
                long delta = cacheTimeout * 1000
                        - (System.currentTimeMillis() - this.creationTime);
                if (delta > 0) {
                    RestServer.timer.schedule(new TimerTask() {
                        @Override
                        public void run() {
                            SipUtils.removeDialogContext(key, DialogContext.this);
                        }

                    }, delta);
                }
            }
        }
    }

    public static DialogContext get(Dialog dialog) {
        return (DialogContext) dialog.getApplicationData();
    }

    /**
     * Timer task that sends a periodic in-dialog Options to test the liveness of the phone on the
     * other side.
     */
    class OptionsTimer extends TimerTask {

        @Override
        public void run() {
            try {
                for (Dialog dialog : dialogs) {
                    if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                        Request request = dialog.createRequest(Request.OPTIONS);
                        SipProvider provider = ((DialogExt) dialog).getSipProvider();
                        ClientTransaction ctx = provider.getNewClientTransaction(request);
                        dialog.sendRequest(ctx);
                    }
                }
            } catch (Exception ex) {
                this.cancel();
                for (Dialog dialog : dialogs) {
                    try {
                        if (dialog != null && dialog.getState() == DialogState.CONFIRMED) {
                            Request request = dialog.createRequest(Request.BYE);
                            SipProvider provider = ((DialogExt) dialog).getSipProvider();
                            ClientTransaction ctx = provider.getNewClientTransaction(request);
                            dialog.sendRequest(ctx);
                        }
                    } catch (SipException e) {
                        logger.error("Unexpected exception sending BYE", e);
                    }
                }
            }

        }

    }

    public void setUserCredentials(UserCredentialHash credentials) {
        this.userCredentials = credentials;
    }

    public UserCredentialHash getCredentials() {
        return this.userCredentials;
    }

    public Request getRequest(Dialog dialog) {
       return this.dialogTable.get(dialog);
    }

   

   
}
