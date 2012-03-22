/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.util.HashMap;

import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipListener;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.address.SipURI;
import javax.sip.header.ToHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;

public final class SipListenerImpl implements SipListener {
    private HashMap<String,AbstractSipListener> sipListeners = new HashMap<String,AbstractSipListener>();
    private static Logger logger = Logger.getLogger(SipListenerImpl.class);
    
    
    final  void addServiceListener(MetaInf metaInf, AbstractSipListener sipListener) {
        this.sipListeners.put(metaInf.getSipConvergenceName(), sipListener);
    }

    @Override
    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        for (AbstractSipListener sipListener : this.sipListeners.values() ) {
            if ( sipListener.isHandled(dialogTerminatedEvent)) {
                sipListener.processDialogTerminated(dialogTerminatedEvent);
            }
        }
    }

    @Override
    public void processIOException(IOExceptionEvent exceptionEvent) {    
        logger.error("IOException ");
    }

    @Override
    public void processRequest(RequestEvent requestEvent) {
       
       for (AbstractSipListener sipListener : this.sipListeners.values() ) {
           if ( sipListener.isHandled(requestEvent)) {
               sipListener.processRequest(requestEvent);
           }
       }
       
    }

    @Override
    public void processResponse(ResponseEvent responseEvent) {
      for (AbstractSipListener sipListener : this.sipListeners.values() ) {
          if ( sipListener.isHandled(responseEvent)) {
              sipListener.processResponse(responseEvent);
          }
      }
    }

    @Override
    public void processTimeout(TimeoutEvent timeoutEvent) {
        for (AbstractSipListener sipListener : this.sipListeners.values() ) {
            if ( sipListener.isHandled(timeoutEvent)) {
                sipListener.processTimeout(timeoutEvent);
            }
        }

    }

    @Override
    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
      
        for (AbstractSipListener sipListener : this.sipListeners.values() ) {
            if ( sipListener.isHandled(transactionTerminatedEvent)) {
                sipListener.processTransactionTerminated(transactionTerminatedEvent);
            }
        }

    }

}
