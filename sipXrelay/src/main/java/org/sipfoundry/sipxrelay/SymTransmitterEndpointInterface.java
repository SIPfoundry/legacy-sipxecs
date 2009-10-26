/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxrelay;


/**
 * Transmitter endpoint for symmitron.
 * 
 * @author mranga
 */
public interface SymTransmitterEndpointInterface extends SymEndpointInterface {
    
    
    
    /**
     * Set the remote endpoint on hold.
     */
    public void setOnHold(boolean flag);

}
