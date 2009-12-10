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
