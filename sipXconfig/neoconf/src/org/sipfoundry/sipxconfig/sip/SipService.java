/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.sip;



/**
 * Sip utility service functions complete w/server current configuration
 */
public interface SipService {
    public void sendCheckSync(String addrSpec);

    public void sendNotify(String addrSpec, String eventType, String contentType, byte[] payload);
    
    public void sendRefer(String sourceAddrSpec, String destinationAddSpec);
}
