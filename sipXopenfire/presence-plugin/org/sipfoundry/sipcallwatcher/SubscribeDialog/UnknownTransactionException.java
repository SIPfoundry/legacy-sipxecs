/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipcallwatcher.SubscribeDialog;

public class UnknownTransactionException extends Exception
{
    public UnknownTransactionException(String message) {
        super(message);
    }
    
    public UnknownTransactionException(String message, Exception ex) {
        super(message,ex);
    }

    public UnknownTransactionException(Exception ex) {
        super(ex);
    }
}
