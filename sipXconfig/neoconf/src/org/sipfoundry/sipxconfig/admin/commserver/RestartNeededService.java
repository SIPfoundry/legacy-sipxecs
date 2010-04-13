/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.Serializable;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.service.SipxConfigService;

public class RestartNeededService implements Serializable {
    private final String m_fqdn;
    private final String m_serviceBeanId;

    public RestartNeededService(String fqdn, String serviceBeanId) {
        m_fqdn = fqdn;
        m_serviceBeanId = serviceBeanId;
    }

    public String getLocation() {
        return m_fqdn;
    }

    public String getServiceBeanId() {
        return m_serviceBeanId;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_fqdn).append(m_serviceBeanId).toHashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (obj instanceof RestartNeededService) {
            RestartNeededService service = (RestartNeededService) obj;
            return m_fqdn.equals(service.getLocation()) && m_serviceBeanId.equals(service.getServiceBeanId());
        }
        return false;
    }

    public boolean isConfigurationRestartNeeded() {
        return StringUtils.equals(m_serviceBeanId, SipxConfigService.BEAN_ID);
    }
}
