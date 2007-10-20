/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.localization;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class Localization extends BeanWithId {

    private String m_region;

    private String m_language;
    
    public String getRegion() {
        return m_region;
    }
    
    public void setRegion(String region) {
        this.m_region = region;
    }

    public String getLanguage() {
        return m_language;
    }
    
    public void setLanguage(String language) {
        this.m_language = language;
    }
    
    
}
