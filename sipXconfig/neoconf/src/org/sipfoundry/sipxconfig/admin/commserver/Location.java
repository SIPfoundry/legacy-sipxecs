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
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.collections.Transformer;
import org.apache.commons.lang.RandomStringUtils;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.nattraversal.NatLocation;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.collections.CollectionUtils.collect;
import static org.apache.commons.collections.CollectionUtils.select;
import static org.apache.commons.collections.CollectionUtils.subtract;
import static org.apache.commons.lang.StringUtils.substringBefore;

public class Location extends BeanWithId {
    // security role
    public static final String ROLE_LOCATION = "ROLE_LOCATION";
    public static final int PROCESS_MONITOR_PORT = 8092;
    private static final int SOFTWARE_ADMIN_PORT = 8092;
    private static final int HTTPS_SERVER_PORT = 8092;
    private static final int LOCATION_PASSWORD_LEN = 8;
    private static final String XML_RPC_URL = "https://%s:%d/RPC2";

    private String m_name;
    private String m_address;
    private String m_fqdn;
    private String m_password = RandomStringUtils.randomAlphanumeric(LOCATION_PASSWORD_LEN);
    private boolean m_primary;
    private boolean m_registered;
    private List<String> m_installedBundles;
    private NatLocation m_nat = new NatLocation();
    private ServerRoleLocation m_serverRoles = new ServerRoleLocation();

    private Collection<LocationSpecificService> m_services;
    private DaoEventPublisher m_daoEventPublisher;
    private Branch m_branch;

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
     * @return beanIDs of installed bundles
     */
    public List<String> getInstalledBundles() {
        return m_installedBundles;
    }

    public void setInstalledBundles(List<String> installedBundles) {
        m_installedBundles = installedBundles;
    }

    public NatLocation getNat() {
        return m_nat;
    }

    public void setNat(NatLocation nat) {
        m_nat = nat;
    }

    public ServerRoleLocation getServerRoles() {
        return m_serverRoles;
    }

    public void setServerRoles(ServerRoleLocation serverRoles) {
        m_serverRoles = serverRoles;
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
        return String.format(XML_RPC_URL, m_fqdn, PROCESS_MONITOR_PORT);
    }

    public String getSoftwareAdminUrl() {
        return String.format(XML_RPC_URL, m_fqdn, SOFTWARE_ADMIN_PORT);
    }

    public String getHttpsServerUrl() {
        return String.format("https://%s:%d", m_fqdn, HTTPS_SERVER_PORT);
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

    public Branch getBranch() {
        return m_branch;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
    }

    /**
     * Set this locations collection of services by via a collection of SipxService definition
     * objects
     *
     * @param services
     */
    public void setServiceDefinitions(Collection< ? extends SipxService> services) {
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
        service.getSipxService().onDestroy();
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
        service.getSipxService().onInit();
    }

    public void addService(SipxService service) {
        LocationSpecificService lss = new LocationSpecificService(service);
        addService(lss);
    }

    public void addServices(Collection< ? extends SipxService> services) {
        for (SipxService sipxService : services) {
            addService(sipxService);
        }
    }

    public void removeServices(Collection< ? extends SipxService> services) {
        Collection<LocationSpecificService> lsServices = getServices();
        if (lsServices == null) {
            return;
        }
        for (Iterator<LocationSpecificService> iterator = lsServices.iterator(); iterator.hasNext();) {
            LocationSpecificService locationSpecificService = iterator.next();
            SipxService service = locationSpecificService.getSipxService();
            if (services.contains(service)) {
                service.onDestroy();
                iterator.remove();
            }
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
        return substringBefore(m_fqdn, ".");
    }

    /**
     * Collect services that are marked for restart after upgrade. Reset 'enableOnNextUpgrade'
     * status for all returned services.
     *
     * Location needs to be saved after calling this method.
     */
    public Collection<SipxService> getToBeEnabledServices() {
        Collection<SipxService> toBeEnabled = new ArrayList<SipxService>();
        for (LocationSpecificService locationSpecificService : getServices()) {
            if (!locationSpecificService.getEnableOnNextUpgrade()) {
                continue;
            }
            SipxService sipxService = locationSpecificService.getSipxService();
            toBeEnabled.add(sipxService);
            locationSpecificService.setEnableOnNextUpgrade(false);
        }

        return toBeEnabled;
    }

    public boolean isPrimary() {
        return m_primary;
    }

    public void setPrimary(boolean primary) {
        m_primary = primary;
    }

    public boolean isRegistered() {
        return m_registered;
    }

    public void setRegistered(boolean registered) {
        m_registered = registered;
    }

    /**
     * Retrieves the list of services installed at this location.
     *
     * For each LocationSpecificService it retrieves the underlying SipxService.
     */
    public Collection<SipxService> getSipxServices() {
        Collection<LocationSpecificService> services = getServices();
        Transformer retrieveService = new Transformer() {
            public Object transform(Object item) {
                LocationSpecificService lss = (LocationSpecificService) item;
                return lss.getSipxService();
            }
        };
        return collect(services, retrieveService, new ArrayList());
    }

    /**
     * If list of installed bundles is empty add all auto-enabled bundles to the list.
     *
     * @return true is bundles have been initialized and location needs to be saved
     */
    public boolean initBundles(SipxServiceManager sipxServiceManager) {
        if (m_installedBundles != null && !m_installedBundles.isEmpty()) {
            // already have bundles...
            return false;
        }
        m_installedBundles = new ArrayList<String>();
        Collection<SipxServiceBundle> bundles = sipxServiceManager.getBundleDefinitions();
        for (SipxServiceBundle bundle : bundles) {
            if (bundle.autoEnableOn(this)) {
                m_installedBundles.add(bundle.getModelId());
            }
        }
        return true;
    }

    /**
     * Filters the list of bundles removing the ones that cannot be installed on this location.
     *
     * @param all bundles
     * @return bundles that can be installed (it may include bundles that are already installed)
     */
    public Collection<SipxServiceBundle> getInstallableBundles(Collection<SipxServiceBundle> all) {
        List<SipxServiceBundle> filtered = new ArrayList<SipxServiceBundle>();
        select(all, new SipxServiceBundle.CanRunOn(this), filtered);
        return filtered;
    }

    public boolean isBundleInstalled(String bundleId) {
        return m_installedBundles == null ? false : m_installedBundles.contains(bundleId);
    }

    /**
     * Reinitialize the list of services based on selected bundles.
     *
     * The bundle <-> service mapping might change as a result of installing new services or
     * upgrading the system. Calling this method will reset the services list based on already
     * selected bundles.
     */
    public void resetBundles(SipxServiceManager sipxServiceManager) {
        Collection<SipxService> oldServices = getSipxServices();
        Collection<SipxService> newServices = sipxServiceManager.getServiceDefinitions(sipxServiceManager
                .getBundlesForLocation(this));

        Collection<SipxService> stopServices = subtract(oldServices, newServices);
        Collection<SipxService> startServices = subtract(newServices, oldServices);

        removeServices(stopServices);
        addServices(startServices);
    }

    public boolean isServiceInstalled(SipxService service) {
        return getSipxServices().contains(service);
    }
}
