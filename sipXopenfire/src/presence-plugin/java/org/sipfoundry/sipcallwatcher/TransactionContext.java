/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher;

import javax.sip.ClientTransaction;
import javax.sip.Transaction;

/**
 * A store for per transaction data.
 * This is for later when we multiplex to a third party call controller
 * based on IM presence. This is where we plan to store the state
 * for the transaction state machine involved with third party 
 * call control. There is a pointer from the transaction to the 
 * data stored here.
 */
public class TransactionContext {
    private Transaction transaction;
    Operator operator;

    private TransactionContext(Transaction transaction) {
        transaction.setApplicationData(this);
        this.transaction = transaction;
    }

    /**
     * @return the transaction
     */
    public Transaction getTransaction() {
        return transaction;
    }

    public static TransactionContext attach(ClientTransaction subscribeTransaction,
            Operator operator) {
        if (subscribeTransaction.getApplicationData() == null) {
            TransactionContext txContext = new TransactionContext(subscribeTransaction);
            txContext.operator = operator;
            return txContext;
        } else {
            TransactionContext txContext = (TransactionContext) subscribeTransaction
                    .getApplicationData();
            assert operator == txContext.operator;
            return txContext;
        }
    }

}
