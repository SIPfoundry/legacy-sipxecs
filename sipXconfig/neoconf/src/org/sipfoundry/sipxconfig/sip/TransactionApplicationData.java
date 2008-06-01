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

import java.util.EventObject;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.TimeUnit;

import javax.sip.ResponseEvent;
import javax.sip.TimeoutEvent;

/**
 * Transaction context data. Register one of these per transaction to track data that is specific
 * to a transaction.
 * 
 */
class TransactionApplicationData {

    private Operator m_operator;

    private SynchronousQueue<EventObject> m_queue = new SynchronousQueue<EventObject>();

  
    public TransactionApplicationData(Operator operator) {
        m_operator = operator;
    }

    /**
     * Blocks until transaction completes or timeout elapses
     * 
     * @return false if timeout, true if response received
     * @throws InterruptedException
     */
    public EventObject block() throws InterruptedException {
        return m_queue.poll(5000, TimeUnit.MILLISECONDS);
      
    }

    public void response(ResponseEvent responseEvent) {
        if (m_operator == Operator.SEND_NOTIFY) {
             // We ignore 1xx responses. 2xx and above are put into the queue.
            if (responseEvent.getResponse().getStatusCode() / 100 >= 2) {
                m_queue.add(responseEvent);
            }
        }
    }

    public void timeout(TimeoutEvent timeoutEvent) {
        // FIXME: we need to report timeout somehow
        if (m_operator == Operator.SEND_NOTIFY) {
            m_queue.add(timeoutEvent);
        }
    }

 
}
