/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in grandstream models
 */
public final class GrandstreamModel extends PhoneModel {

    private boolean m_isHandyTone;
    
    public GrandstreamModel() {
        setBeanId(GrandstreamPhone.BEAN_ID);
    }
    
    public void setIsHandyTone(boolean isHandyTone) {
        m_isHandyTone = isHandyTone;
    }

    public boolean isHandyTone() {
        return m_isHandyTone;
    }
}
