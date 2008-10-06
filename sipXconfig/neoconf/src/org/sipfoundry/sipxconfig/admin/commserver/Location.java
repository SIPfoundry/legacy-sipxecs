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
    private static final String HTTP_PREFIX = "https://";
    private static final int PROCESS_MONITOR_PORT = 8092;
    private static final String PROCESS_MONITOR_PATH = "/RPC2";
    private static final String EMPTY_STRING = "";

    private String m_name;
    private String m_sipDomain;
    private String m_address;
    private String m_fqdn;

    private String m_domainname;
    private String m_hostname;

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
        if (m_fqdn != null) {
            int i = m_fqdn.indexOf('.');
            if (i != -1) {
                m_hostname = m_fqdn.substring(0, i);
                m_domainname = m_fqdn.substring(i + 1);
            } else {
                m_hostname = m_fqdn;
                m_domainname = EMPTY_STRING;
            }
        } else {
            m_hostname = EMPTY_STRING;
            m_domainname = EMPTY_STRING;
        }
    }

    public String getDomainname() {
        return m_domainname;
    }

    public String getHostname() {
        return m_hostname;
    }

    /**
     * Sets this instances address field based on the value parsed from the given URL.  For
     * example, the URL of "https://localhost:8091/cgi-bin/replication/replication.cgi" will
     * result in an address value of "localhost"
     * @param url The URL to parse, either the process monitor url or the replication url
     */
    public void setUrl(String url) {
        Pattern addressPattern = Pattern.compile("^http[s]?://([a-zA-Z0-9\\.]+):.*$");
        Matcher matcher = addressPattern.matcher(url);
        matcher.matches();
        String address = matcher.group(1);
        if (isValidIp(address)) {
            setAddress(address);
        } else {
            setFqdn(address);
        }
    }

    private static boolean isValidIp(final String ip) {
        return ip.matches("[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}\\.[\\d]{1,3}$");
    }

    public String getProcessMonitorUrl() {
        if ((m_fqdn != null) && (m_fqdn.length() != 0)) {
            return HTTP_PREFIX + m_fqdn + ':' + PROCESS_MONITOR_PORT + PROCESS_MONITOR_PATH;
        } else if (m_address != null) {
            return HTTP_PREFIX + m_address + ':' + PROCESS_MONITOR_PORT + PROCESS_MONITOR_PATH;
        } else {
            return EMPTY_STRING;
        }
    }

    public String getSipDomain() {
        return m_sipDomain;
    }

    public void setSipDomain(String sipDomain) {
        m_sipDomain = sipDomain;
    }

    public void setSipxServices(Collection<SipxService> sipxServices) {
        m_sipxServices = sipxServices;
    }

    /**
     * Returns an unmodifiable collection of sipx services for this location.  To add or remove
     * services on this location, use the Location's addService or removeService methods.
     */
    public Collection<SipxService> getSipxServices() {
        if (m_sipxServices == null) {
            return Collections.unmodifiableCollection(Collections.<SipxService>emptyList());
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
