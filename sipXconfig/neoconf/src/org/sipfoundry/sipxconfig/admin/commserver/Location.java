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
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.service.SipxService;

public class Location extends BeanWithId {
    private static final String HTTP_PREFIX = "https://";
    private static final int PROCESS_MONITOR_PORT = 8092;
    private static final String PROCESS_MONITOR_PATH = "/RPC2";
    
    private String m_name;
    private String m_sipDomain;
    private String m_address;
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
        setAddress(address);
    }
    
    public String getProcessMonitorUrl() {
        return HTTP_PREFIX + m_address + ':' + PROCESS_MONITOR_PORT + PROCESS_MONITOR_PATH;
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
    
    public Collection<SipxService> getSipxServices() {
        return m_sipxServices;
    }
}
