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
import java.util.Collection;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.RandomStringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.springframework.beans.factory.annotation.Required;

public class Location extends BeanWithId {
    // security role
    public static final String ROLE_LOCATION = "ROLE_LOCATION";
    private static final int PROCESS_MONITOR_PORT = 8092;
    private static final int LOCATION_PASSWORD_LEN = 8;

    private String m_name;
    private String m_address;
    private String m_fqdn;
    private String m_password = RandomStringUtils.randomAlphanumeric(LOCATION_PASSWORD_LEN);
    private boolean m_primary;

    private Collection<LocationSpecificService> m_services;
    private DaoEventPublisher m_daoEventPublisher;

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

    @Required
    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
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

    /**
     * Set this locations collection of services by directly supplying the LocationSpecificService
     * objects
     *
     * @param services
     */
    public void setServices(Collection<LocationSpecificService> services) {
        m_services = services;
    }

    /**
     * Set this locations collection of services by via a collection of SipxService definition
     * objects
     *
     * @param services
     */
    public void setServiceDefinitions(Collection<SipxService> services) {
        m_services = new ArrayList<LocationSpecificService>();
        for (SipxService sipxService : services) {
            LocationSpecificService newService = new LocationSpecificService();
            newService.setSipxService(sipxService);
            addService(newService);
        }
    }

    /**
     * Returns an unmodifiable collection of sipx services for this location. To add or remove
     * services on this location, use the Location's addService or removeService methods.
     */
    public Collection<LocationSpecificService> getServices() {
        return m_services;
    }

    public LocationSpecificService getService(String beanId) {
        LocationSpecificService serviceForBeanId = null;

        if (m_services != null) {
            Iterator<LocationSpecificService> iterator = m_services.iterator();
            while (iterator.hasNext() && serviceForBeanId == null) {
                LocationSpecificService next = iterator.next();
                SipxService service = next.getSipxService();
                if (beanId.equals(service.getBeanId())) {
                    serviceForBeanId = next;
                }
            }         
        }
        
        return serviceForBeanId;
    }
    
    public void removeService(LocationSpecificService service) {
        if (m_daoEventPublisher != null) {
            m_daoEventPublisher.publishDelete(service);
        }
        m_services.remove(service);
    }

    public void removeServiceByBeanId(String beanId) {
        LocationSpecificService service = getService(beanId);
        removeService(service);
    }

    public void addService(LocationSpecificService service) {
        if (m_services == null) {
            m_services = new ArrayList<LocationSpecificService>();
        }
        String serviceName = service.getSipxService().getBeanId();
        for (LocationSpecificService lss : m_services) {
            if (serviceName.equals(lss.getSipxService().getBeanId())) {
                return;
            }
        }
        service.setLocation(this);
        m_services.add(service);
    }

    public void addService(SipxService service) {
        LocationSpecificService lss = new LocationSpecificService(service);
        addService(lss);
    }

    public void addServices(Collection<SipxService> services) {
        for (SipxService sipxService : services) {
            addService(sipxService);
        }
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }

    /**
     * Get the hostname portion of the fully qualified domain name
     */
    public String getHostname() {
        String fqdn = getFqdn();
        if (fqdn.indexOf('.') < 0) {
            return fqdn;
        } else {
            String hostname = fqdn.substring(0, fqdn.indexOf('.'));
            return hostname;
        }
    }

    public boolean isPrimary() {
        return m_primary;
    }

    public void setPrimary(boolean primary) {
        m_primary = primary;
    }

}
