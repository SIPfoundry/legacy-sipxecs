/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;
import java.util.Map;

public class DefaultExternalCommandContext implements ExternalCommandContext, Serializable {

    private String m_binDirectory;
    private Map<String, String> m_argResolverMap;

    public String getBinDirectory() {
        return m_binDirectory;
    }
    
    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public void setArgumentResolverMap(Map<String, String> argResolverMap) {
        m_argResolverMap = argResolverMap;
    }
    
    public String resolveArgumentString(String key) {
        return m_argResolverMap.get(key);
    }
}
