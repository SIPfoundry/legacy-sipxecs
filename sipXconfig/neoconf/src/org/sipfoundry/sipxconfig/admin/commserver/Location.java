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

import java.util.Collection;
import java.util.Collections;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.service.SipxService;

public class Location extends BeanWithId {
    private static final int PROCESS_MONITOR_PORT = 8092;

    private String m_name;
    private String m_address;
    private String m_fqdn;

    private Collection<SipxService> m_sipxServices;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getFqdn() {
        return m_fqdn;
    }

    public void setFqdn(String fqdn) {
        m_fqdn = fqdn;
    }

    /**
     * Sets this instances address field based on the value parsed from the given URL. For
     * example, the URL of "https://localhost:8091/cgi-bin/replication/replication.cgi" will
     * result in an address value of "localhost"
     *
     * This method is used during migrating from old topology.xml format only.
     *
     * @param url The URL to parse, either the process monitor url or the replication url
     */
    @Deprecated
    public void setUrl(String url) {
        Pattern addressPattern = Pattern.compile("^http[s]?://([a-zA-Z0-9\\.]+):.*$");
        Matcher matcher = addressPattern.matcher(url);
        matcher.matches();
        String address = matcher.group(1);
        setFqdn(address);
    }


    public String getProcessMonitorUrl() {
        return String.format("https://%s:%d/RPC2", m_fqdn, PROCESS_MONITOR_PORT);
    }

    public void setSipxServices(Collection<SipxService> sipxServices) {
        m_sipxServices = sipxServices;
    }

    /**
     * Returns an unmodifiable collection of sipx services for this location. To add or remove
     * services on this location, use the Location's addService or removeService methods.
     */
    public Collection<SipxService> getSipxServices() {
        if (m_sipxServices == null) {
            return Collections.unmodifiableCollection(Collections.<SipxService> emptyList());
        }
        return Collections.unmodifiableCollection(m_sipxServices);
    }

    public void removeService(SipxService sipxService) {
        m_sipxServices.remove(sipxService);
    }

    public void addService(SipxService sipxService) {
        if (!m_sipxServices.contains(sipxService)) {
            m_sipxServices.add(sipxService);
        }
    }
}
