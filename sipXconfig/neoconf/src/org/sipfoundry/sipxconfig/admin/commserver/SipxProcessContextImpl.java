/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext.PROCESS_STATE_CHANGE;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Running;

public class SipxProcessContextImpl implements SipxProcessContext {
    private static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);

    private LocationsManager m_locationsManager;
    private ApiProvider<ProcessManagerApi> m_processManagerApiProvider;
    private SipxServiceManager m_sipxServiceManager;
    private DialPlanActivationManager m_dialPlanActivationManager;
    private AuditLogContext m_auditLogContext;
    private RestartNeededState m_servicesToRestart = new RestartNeededState();
    private ReloadNeededState m_servicesToReload = new ReloadNeededState();

    /** list of services that should trigger dial plan replication if restarted */
    private final Set<String> m_replicateDialPlanBeforeRestartServices = new HashSet<String>();

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setProcessManagerApiProvider(ApiProvider processManagerApiProvider) {
        m_processManagerApiProvider = processManagerApiProvider;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    public boolean needsRestart(Location location, SipxService service) {
        return m_servicesToRestart.isMarked(location, service);
    }

    public boolean needsRestart() {
        return !m_servicesToRestart.isEmpty();
    }

    public boolean needsReload(Location location, SipxService service) {
        return m_servicesToReload.isMarked(location, service);
    }

    public boolean needsReload() {
        return !m_servicesToReload.isEmpty();
    }

    public void replicateDialPlanBeforeRestart(String serviceBeanId) {
        m_replicateDialPlanBeforeRestartServices.add(serviceBeanId);
    }

    /**
     * Read service status values from the process monitor and return them in an array.
     * ClassCastException or NoSuchElementException (both RuntimeException subclasses) could be
     * thrown from this method, but only if things have gone horribly wrong.
     *
     * @param onlyActiveServices If true, only return status information for the services that the
     *        location parameter lists in its services list. If false, return all service status
     *        information available.
     */
    public ServiceStatus[] getStatus(Location location, boolean onlyActiveServices) {
        try {
            // Break the result into the keys and values.
            ProcessManagerApi api = m_processManagerApiProvider.getApi(location.getProcessMonitorUrl());
            Map<String, String> result = api.getStateAll(getHost());
            return extractStatus(result, location, onlyActiveServices);
        } catch (XmlRpcRemoteException e) {
            throw new UserException("&xml.rpc.error.state", location.getFqdn());
        }
    }

    public ServiceStatus.Status getStatus(Location location, SipxService service) {
        String serviceBeanId = service.getBeanId();
        ServiceStatus[] status = getStatus(location, true);
        for (ServiceStatus serviceStatus : status) {
            if (serviceBeanId.equals(serviceStatus.getServiceBeanId())) {
                return serviceStatus.getStatus();
            }
        }
        return ServiceStatus.Status.Undefined;
    }

    public List<String> getStatusMessages(Location location, SipxService service) {
        try {
            ProcessManagerApi api = m_processManagerApiProvider.getApi(location.getProcessMonitorUrl());
            return api.getStatusMessages(getHost(), service.getProcessName());
        } catch (XmlRpcRemoteException e) {
            throw new UserException("&xml.rpc.error.status.messages", location.getFqdn());
        }
    }

    /**
     * Loop through the key-value pairs and construct the ServiceStatus.
     */
    private ServiceStatus[] extractStatus(Map<String, String> statusAll, Location location,
            boolean onlyActiveServices) {
        List<String> locationServiceProcesses = new ArrayList<String>();
        if (onlyActiveServices) {
            for (LocationSpecificService service : location.getServices()) {
                locationServiceProcesses.add(service.getSipxService().getProcessName());
            }
        }

        List<ServiceStatus> serviceStatusList = new ArrayList(statusAll.size());
        for (Map.Entry<String, String> entry : statusAll.entrySet()) {
            String name = entry.getKey();

            SipxService service = m_sipxServiceManager.getServiceByName(name);
            if (service == null) {
                // ignore unknown services
                LOG.warn("Unknown process name " + name + " received from: " + location.getProcessMonitorUrl());
                continue;
            }

            if (onlyActiveServices && !locationServiceProcesses.contains(name)) {
                // ignore services not running at specified location
                continue;
            }

            boolean needsRestart = needsRestart(location, service);
            boolean needsReload = needsReload(location, service);
            ServiceStatus status = new ServiceStatus(service.getBeanId(), entry.getValue(), needsRestart,
                    needsReload);
            serviceStatusList.add(status);
        }

        return serviceStatusList.toArray(new ServiceStatus[serviceStatusList.size()]);
    }

    /**
     * Mark services for restart for all locations.
     *
     * Only services attached to a location are marked for restart
     */
    public void markServicesForRestart(Collection< ? extends SipxService> processes) {
        for (Location location : m_locationsManager.getLocations()) {
            markServicesForRestart(location, processes);
        }
    }

    public void markServicesForRestart(Location location, Collection< ? extends SipxService> processes) {
        for (SipxService service : processes) {
            if (location.isServiceInstalled(service) && location.isRegistered()) {
                m_servicesToRestart.mark(location, service);
            }
        }
    }

    public void manageServices(Collection< ? extends SipxService> processes, Command command) {
        for (Location location : m_locationsManager.getLocations()) {
            manageServices(location, processes, command);
        }
    }

    public void manageServices(Map<Location, List<SipxService>> servicesMap, Command command) {
        Location configLocation = null;
        List<SipxService> restartServicesLast = null;
        SipxService configService = m_sipxServiceManager.getServiceByBeanId(SipxConfigService.BEAN_ID);
        for (Location location : servicesMap.keySet()) {
            List<SipxService> restartServices = servicesMap.get(location);
            // remember only the first location where configuration service is found
            // anyway, config service runs only on one location
            if (!restartServices.contains(configService) || configLocation != null) {
                manageServices(location, restartServices, command);
            } else {
                configLocation = location;
                restartServicesLast = restartServices;
            }
        }
        // restart services from config location last
        if (configLocation != null && restartServicesLast != null) {
            manageServices(configLocation, restartServicesLast, command);
        }
    }

    private void replicateDialPlanBeforeRestart(Collection< ? extends SipxService> processes) {
        for (SipxService process : processes) {
            if (m_replicateDialPlanBeforeRestartServices.contains(process.getBeanId())) {
                m_dialPlanActivationManager.replicateIfNeeded();
                break;
            }
        }
        m_replicateDialPlanBeforeRestartServices.clear();
    }

    public void manageServices(Location location, Collection< ? extends SipxService> processes, Command command) {
        if (processes.isEmpty()) {
            return;
        }
        if (!location.isRegistered()) {
            return;
        }
        // any command: start, stop, restart effectively triggers a dial plan replication (if
        // needed).
        if (!command.equals(Command.RELOAD)) {
            replicateDialPlanBeforeRestart(processes);
        }

        try {
            String[] processNames = new String[processes.size()];
            int i = 0;
            for (SipxService process : processes) {
                processNames[i++] = process.getProcessName();
            }

            ProcessManagerApi api = m_processManagerApiProvider.getApi(location.getProcessMonitorUrl());
            switch (command) {
            case RESTART:
                api.restart(getHost(), processNames, true);
                logProcessStateChange(processNames, location, PROCESS_STATE_CHANGE.RESTARTED);

                for (SipxService process : processes) {
                    process.onRestart();
                }
                m_servicesToRestart.unmark(location, processes);

                break;
            case START:
                api.start(getHost(), processNames, true);
                logProcessStateChange(processNames, location, PROCESS_STATE_CHANGE.STARTED);

                for (SipxService process : processes) {
                    process.onStart();
                }
                m_servicesToRestart.unmark(location, processes);

                break;
            case STOP:
                api.stop(getHost(), processNames, true);
                logProcessStateChange(processNames, location, PROCESS_STATE_CHANGE.STOPPED);

                for (SipxService process : processes) {
                    process.onStop();
                }
                m_servicesToRestart.unmark(location, processes);

                break;
            case RELOAD:
                logProcessStateChange(processNames, location, PROCESS_STATE_CHANGE.RELOADED);

                for (SipxService process : processes) {
                    process.onReload();
                }

                break;
            default:
                break;
            }
            // any command: start, stop, restart effectively clears need for reload..
            m_servicesToReload.unmark(location, processes);
        } catch (XmlRpcRemoteException e) {
            throw new UserException("&xml.rpc.error.operation", location.getFqdn());
        }
    }

    private void logProcessStateChange(String[] processNames, Location location, PROCESS_STATE_CHANGE stateChange) {
        for (String processName : processNames) {
            m_auditLogContext.logProcessStateChange(stateChange, processName, location);
        }
    }

    public LocationStatus getLocationStatus(Location location) {
        Collection<SipxService> sipxServices = location.getSipxServices();
        Map<String, SipxService> beanIdToService = new HashMap();
        for (SipxService sipxService : sipxServices) {
            beanIdToService.put(sipxService.getBeanId(), sipxService);
        }
        ServiceStatus[] statuses = getStatus(location, false);
        Collection<SipxService> toBeStarted = new ArrayList<SipxService>();
        Collection<SipxService> toBeStopped = new ArrayList<SipxService>();
        for (ServiceStatus status : statuses) {
            String serviceBeanId = status.getServiceBeanId();
            boolean shouldBeRunning = beanIdToService.containsKey(serviceBeanId);
            boolean isRunningNow = status.getStatus() == Running;
            if (shouldBeRunning && !isRunningNow) {
                toBeStarted.add(m_sipxServiceManager.getServiceByBeanId(serviceBeanId));
            }
            if (isRunningNow && !shouldBeRunning) {
                toBeStopped.add(m_sipxServiceManager.getServiceByBeanId(serviceBeanId));
            }
        }
        return new LocationStatus(toBeStarted, toBeStopped);
    }

    private String getHost() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public Collection<RestartNeededService> getRestartNeededServices() {
        return m_servicesToRestart.getAffected();
    }

    public void restartMarkedServices(Location location) {
        Collection<String> beanIds = m_servicesToRestart.getServices(location);
        if (beanIds.isEmpty()) {
            return;
        }
        List<SipxService> services = new ArrayList<SipxService>(beanIds.size());
        for (String beanId : beanIds) {
            SipxService service = m_sipxServiceManager.getServiceByBeanId(beanId);
            services.add(service);
        }
        manageServices(location, services, Command.RESTART);
    }

    public void clear() {
        m_servicesToRestart = new RestartNeededState();
        m_servicesToReload = new ReloadNeededState();
        m_replicateDialPlanBeforeRestartServices.clear();
    }

    public void markDialPlanRelatedServicesForRestart(String... serviceBeanIds) {
        Collection<SipxService> services = new ArrayList<SipxService>();
        for (String serviceBeanId : serviceBeanIds) {
            replicateDialPlanBeforeRestart(serviceBeanId);
            services.add(m_sipxServiceManager.getServiceByBeanId(serviceBeanId));
        }
        markServicesForRestart(services);
    }

    public void unmarkServicesToRestart(Collection<RestartNeededService> services) {
        m_servicesToRestart.unmark(services);
    }

    public void markServicesForReload(Collection< ? extends SipxService> processes) {
        for (Location location : m_locationsManager.getLocations()) {
            markServicesForReload(location, processes);
        }
    }

    private void markServicesForReload(Location location, Collection< ? extends SipxService> processes) {
        for (SipxService service : processes) {
            if (location.isServiceInstalled(service) && location.isRegistered()) {
                m_servicesToReload.mark(location, service);
            }
        }
    }

    @Override
    public Collection<ReloadNeededService> getReloadNeededServices() {
        return m_servicesToReload.getAffected();
    }

    @Override
    public void reloadMarkedServices(Location location) {
        Collection<String> beanIds = m_servicesToReload.getServices(location);
        if (beanIds.isEmpty()) {
            return;
        }
        List<SipxService> services = new ArrayList<SipxService>(beanIds.size());
        for (String beanId : beanIds) {
            SipxService service = m_sipxServiceManager.getServiceByBeanId(beanId);
            services.add(service);
        }
        manageServices(location, services, Command.RELOAD);
    }

    @Override
    public void unmarkServicesToReload(Collection<ReloadNeededService> services) {
        m_servicesToReload.unmark(services);
    }
}
