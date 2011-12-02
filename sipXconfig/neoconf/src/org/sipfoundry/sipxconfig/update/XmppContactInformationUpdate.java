/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.update;

import org.sipfoundry.sipxconfig.admin.commserver.ContactInfoChangeApi;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

public class XmppContactInformationUpdate {
    private ApiProvider<ContactInfoChangeApi> m_contactInfoChangeApiProvider;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private SipxProcessContext m_sipxProcessContext;

    @Required
    public void setContactInfoChangeApiProvider(ApiProvider<ContactInfoChangeApi> contactInfoChangeApiProvider) {
        m_contactInfoChangeApiProvider = contactInfoChangeApiProvider;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSipxProcessContext(SipxProcessContext sipxProcessContext) {
        m_sipxProcessContext = sipxProcessContext;
    }

    public void notifyChange(User user) {
        ImAccount imAccount = new ImAccount(user);
        SipxService openfireService = m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        if (imAccount.isEnabled()) {
            for (Location location : m_locationsManager.getLocationsForService(openfireService)) {
                if (ServiceStatus.Status.Running == m_sipxProcessContext.getStatus(location, openfireService)) {
                    getApi(location).notifyContactChange(imAccount.getImId());
                }
            }
        }
    }

    private ContactInfoChangeApi getApi(Location location) {
        return m_contactInfoChangeApiProvider.getApi(location.getXmppContactInfoUpdateUrl());
    }
}
