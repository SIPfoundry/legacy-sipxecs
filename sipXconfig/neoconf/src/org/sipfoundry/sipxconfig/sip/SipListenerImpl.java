/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

import javax.sip.ClientTransaction;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipListener;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;

class SipListenerImpl implements SipListener {

    public void processDialogTerminated(DialogTerminatedEvent dialogTerminatedEvent) {
        // TODO Auto-generated method stub

    }

    public void processIOException(IOExceptionEvent exceptionEvent) {

    }

    public void processRequest(RequestEvent requestEvent) {
        // TODO Auto-generated method stub
    }

    public void processResponse(ResponseEvent responseEvent) {
        ClientTransaction clientTransaction = responseEvent.getClientTransaction();
        TransactionApplicationData tad = (TransactionApplicationData) clientTransaction.getApplicationData();
        if (tad != null) {
            tad.response(responseEvent);
        }
    }

    public void processTimeout(TimeoutEvent timeoutEvent) {
        if (!timeoutEvent.isServerTransaction()) {
            ClientTransaction clientTransaction = timeoutEvent.getClientTransaction();

            TransactionApplicationData tad = (TransactionApplicationData) clientTransaction.getApplicationData();
            if (tad != null) {
                tad.timeout(timeoutEvent);
            }
        }
    }

    public void processTransactionTerminated(TransactionTerminatedEvent transactionTerminatedEvent) {
        // TODO Do any transaction specific continuation actions here.
    }
}
