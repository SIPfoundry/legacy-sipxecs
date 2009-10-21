package org.sipfoundry.sipcallwatcher.SubscribeDialog;

public class SubscribeDialogStateException extends Exception
{
    public SubscribeDialogStateException(String message) {
        super(message);
    }
    
    public SubscribeDialogStateException(String message, Exception ex) {
        super(message,ex);
    }

    public SubscribeDialogStateException(Exception ex) {
        super(ex);
    }
}

