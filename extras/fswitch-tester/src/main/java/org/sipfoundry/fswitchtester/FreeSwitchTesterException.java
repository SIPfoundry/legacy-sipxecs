/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;

public class FreeSwitchTesterException extends RuntimeException {
    
    public FreeSwitchTesterException ( Exception ex) {
        super(ex);
    }
    
    public FreeSwitchTesterException ( String message, Exception ex) {
        super( message, ex );
    }
    
    public FreeSwitchTesterException ( String message) {
        super(message);
    }

}
