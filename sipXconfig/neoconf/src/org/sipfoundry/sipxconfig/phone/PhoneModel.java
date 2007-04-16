/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;


import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class PhoneModel extends DeviceDescriptor {

    private static final int DEFAULT_MAX_LINES = 4;

    private int m_maxLineCount = DEFAULT_MAX_LINES;
    
    public PhoneModel() {        
    }
    
    public PhoneModel(String beanId) {
        super(beanId);
    }
    
    public PhoneModel(String beanId, String modelId) {
        super(beanId, modelId);
    }

    public void setMaxLineCount(int maxLineCount) {
        m_maxLineCount = maxLineCount;
    }

    public int getMaxLineCount() {
        return m_maxLineCount;
    }
}
