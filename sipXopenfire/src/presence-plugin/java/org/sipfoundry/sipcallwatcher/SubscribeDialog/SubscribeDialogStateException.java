/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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

