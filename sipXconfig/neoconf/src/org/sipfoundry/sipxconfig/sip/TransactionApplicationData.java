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

import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import javax.sip.ResponseEvent;

/**
 * Transaction context data. Register one of these per transaction to track data that is specific
 * to a transaction.
 * 
 */
class TransactionApplicationData {

    private Operator m_operator;

    private Semaphore m_sem = new Semaphore(0);

    private ResponseEvent m_responseEvent;

    public TransactionApplicationData(Operator operator) {
        m_operator = operator;
    }

    /**
     * Blocks until transaction completes or timeout elapses
     * 
     * @return false if timeout, true if response received
     * @throws InterruptedException
     */
    public boolean block() throws InterruptedException {
        return m_sem.tryAcquire(1000, TimeUnit.MILLISECONDS);
    }

    public void response(ResponseEvent responseEvent) {
        if (m_operator == Operator.SEND_NOTIFY) {
            m_responseEvent = responseEvent;
            // FIXME: what are we doing if there is an error?
            if (m_responseEvent.getResponse().getStatusCode() / 100 >= 2) {
                m_sem.release();
            }
        }
    }

    public void timeout() {
        // FIXME: we need to report timeout somehow
        if (m_operator == Operator.SEND_NOTIFY) {
            m_sem.release();
        }
    }
}
