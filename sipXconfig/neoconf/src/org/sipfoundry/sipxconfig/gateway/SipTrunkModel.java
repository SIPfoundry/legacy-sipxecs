/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;


public class SipTrunkModel extends GatewayModel {
    
    public SipTrunkModel() {
        setBeanId(SipTrunk.BEAN_ID);
    }

    /** SIP trunks don't have serial numbers, so return false */
    @Override
    public boolean getHasSerialNumber() {
        return false;
    }

}
