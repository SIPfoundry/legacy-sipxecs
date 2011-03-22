/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.Serializable;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.service.SipxConfigService;

public class ReloadNeededService implements Serializable {
    private final String m_fqdn;
    private final String m_serviceBeanId;

    public ReloadNeededService(String fqdn, String serviceBeanId) {
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
        if (obj instanceof ReloadNeededService) {
            ReloadNeededService service = (ReloadNeededService) obj;
            return m_fqdn.equals(service.getLocation()) && m_serviceBeanId.equals(service.getServiceBeanId());
        }
        return false;
    }

    public boolean isConfigurationReloadNeeded() {
        return StringUtils.equals(m_serviceBeanId, SipxConfigService.BEAN_ID);
    }
}
