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

    /**
     * Used to test if phone supports external lines.
     * 
     * External line is a line that registers with a different SIP proxy than out default proxy.
     * Some phones support multiple lines but all of the have to be registered with the same
     * proxy. This flag needs to be switched to falso for such phones.
     * 
     */
    private boolean m_externalLinesSupported = true;

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

    public void setExternalLinesSupported(boolean externalLinesSupported) {
        m_externalLinesSupported = externalLinesSupported;
    }

    public boolean isExternalLinesSupported() {
        return m_externalLinesSupported;
    }
}
