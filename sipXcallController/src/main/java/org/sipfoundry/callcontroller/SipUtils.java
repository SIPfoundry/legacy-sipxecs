/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.callcontroller;

import gov.nist.javax.sip.DialogExt;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.header.HeaderFactoryImpl;
import gov.nist.javax.sip.header.extensions.ReferredByHeader;

import java.text.ParseException;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Random;
import java.util.Timer;

import javax.sdp.SessionDescription;
import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogState;
import javax.sip.InvalidArgumentException;
import javax.sip.ListeningPoint;
import javax.sip.SipException;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.TransactionUnavailableException;
import javax.sip.address.Address;
import javax.sip.address.AddressFactory;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;
import javax.sip.header.AllowHeader;
import javax.sip.header.CSeqHeader;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ContactHeader;
import javax.sip.header.ContentTypeHeader;
import javax.sip.header.EventHeader;
import javax.sip.header.FromHeader;
import javax.sip.header.Header;
import javax.sip.header.MaxForwardsHeader;
import javax.sip.header.ReferToHeader;
import javax.sip.header.RouteHeader;
import javax.sip.header.ToHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Message;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.Appender;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.jainsip.AbstractSipStackBean;
import org.sipfoundry.commons.jainsip.ListeningPointAddress;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.sipxrest.RestServer;
import org.sipfoundry.sipxrest.SipHelper;
import org.sipfoundry.sipxrest.SipStackBean;

public class SipUtils  {
    

    private static final Logger logger = Logger.getLogger(SipUtils.class);

 	
    private static SipUtils instance;
    
    
    
    private HashMap<String,DialogContext> dialogContextTable = 
        new HashMap<String,DialogContext>();
    
    
    private SipUtils () {
       
    }
  
    public static SipUtils getInstance() {
        if (instance == null ) {
            instance = new SipUtils();
           
        }
        return instance;
    }
    


    /**
     * We set up a timer to terminate the INVITE dialog if we do not see a 200 OK in the transfer.
     *
     * @param dialog dialog to terminate
     */
    public void scheduleTerminate(Dialog dialog) {
        ReferTimerTask referTimerTask = new ReferTimerTask(dialog);
        RestServer.timer.schedule(referTimerTask, 180000);
    }


   
  
   
 
    public synchronized DialogContext createDialogContext(String key) {
        logger.debug("createDialogCOntext " + key);
        if (dialogContextTable.get(key) == null) {
            DialogContext dialogContext = new DialogContext();
            dialogContext.setKey(key);
            this.dialogContextTable.put(key, dialogContext);
        }
        return dialogContextTable.get(key);
    }
    
    public synchronized void removeDialogContext(String key ) {
        this.dialogContextTable.remove(key);
    }
    
    public synchronized DialogContext getDialogContext(String key ) {
        return this.dialogContextTable.get(key);
    }
    public static String formatWithIpAddress(String format) {
        String ipAddress = RestServer.getRestServerConfig().getIpAddress();
        return String.format(format, ipAddress);
    }



  
   
}
