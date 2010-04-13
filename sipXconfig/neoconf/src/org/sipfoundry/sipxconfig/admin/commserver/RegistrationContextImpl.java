/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ImdbApi;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);

    private LocationsManager m_locationsManager;

    private ApiProvider<ImdbApi> m_imdbApiProvider;

    /**
     * @see org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext#getRegistrations()
     */
    public List<RegistrationItem> getRegistrations() {
        try {
            Location primaryProxyLocation = m_locationsManager.getLocationByBundle("primarySipRouterBundle");
            if (primaryProxyLocation == null) {
                LOG.error("No primary proxy found.");
                return Collections.emptyList();
            }
            ImdbApi imdb = m_imdbApiProvider.getApi(primaryProxyLocation.getProcessMonitorUrl());

            Location managementLocation = m_locationsManager.getLocationByBundle("managementBundle");
            if (managementLocation == null) {
                LOG.error("No management bundle found");
                return Collections.emptyList();
            }
            List<Map<String, ? >> registrations = imdb.read(managementLocation.getFqdn(), "registration");
            return getRegistrations(registrations);
        } catch (XmlRpcRemoteException e) {
            // we are handling this separately - server returns FileNotFound even if everything is
            // OK but we have no registrations present
            LOG.warn("Cannot retrieve registrations.", e);
            return Collections.emptyList();
        }
    }

    public List<RegistrationItem> getRegistrationsByUser(User user) {
        return getRegistrationsByUser(getRegistrations(), user);
    }

    List<RegistrationItem> getRegistrations(List<Map<String, ? >> registrations) {
        List<RegistrationItem> items = new ArrayList<RegistrationItem>(registrations.size());
        for (Map<String, ? > r : registrations) {
            RegistrationItem item = new RegistrationItem();
            item.setContact((String) r.get("contact"));
            item.setPrimary((String) r.get("primary"));
            item.setExpires((Integer) r.get("expires"));
            item.setUri((String) r.get("uri"));
            item.setInstrument((String) r.get("instrument"));
            items.add(item);
        }
        return items;
    }

    List<RegistrationItem> getRegistrationsByUser(List<RegistrationItem> registrations, User user) {
        List<RegistrationItem> result = new ArrayList<RegistrationItem>();
        for (RegistrationItem registration : registrations) {
            if (SipUri.extractUser(registration.getUri()).equals(user.getUserName())) {
                result.add(registration);
            }
        }
        return result;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setImdbApiProvider(ApiProvider<ImdbApi> imdbApiProvider) {
        m_imdbApiProvider = imdbApiProvider;
    }
}
