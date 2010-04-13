/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.springframework.beans.factory.annotation.Required;

public abstract class SipxServiceConfiguration extends TemplateConfigurationFile {
    private SipxServiceManager m_sipxServiceManager;

    protected final SipxService getService(String beanId) {
        return m_sipxServiceManager.getServiceByBeanId(beanId);
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public SipxServiceManager getSipxServiceManager() {
        return m_sipxServiceManager;
    }
}
