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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.Factory;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Running;

public class SipxProcessContextImpl implements SipxProcessContext, ApplicationListener {

    private static final Log LOG = LogFactory.getLog(SipxProcessContextImpl.class);

    private final EventsToServices<SipxService> m_eventsToServices = new EventsToServices<SipxService>();
    private String m_host;
    private LocationsManager m_locationsManager;
    private ApiProvider<ProcessManagerApi> m_processManagerApiProvider;
    private SipxServiceManager m_sipxServiceManager;

    @Required
    public void setHost(String host) {
        m_host = host;
    }

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
            Map<String, String> result = api.getStateAll(m_host);
            return extractStatus(result, location, onlyActiveServices);
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e.getCause());
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

            String status = entry.getValue();
            ServiceStatus.Status st = ServiceStatus.Status.valueOf(status);
            if (st == null) {
                st = ServiceStatus.Status.Undefined;
            }

            serviceStatusList.add(new ServiceStatus(service.getBeanId(), st));
        }

        return serviceStatusList.toArray(new ServiceStatus[serviceStatusList.size()]);
    }

    public void manageServices(Collection< ? extends SipxService> processes, Command command) {
        Location[] locations = m_locationsManager.getLocations();
        for (int i = 0; i < locations.length; i++) {
            Location location = locations[i];
            manageServices(location, processes, command);
        }
    }

    public void manageServices(Location location, Collection< ? extends SipxService> processes, Command command) {
        if (processes.isEmpty()) {
            return;
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
                api.restart(m_host, processNames, true);
                break;
            case START:
                api.start(m_host, processNames, true);
                break;
            case STOP:
                api.stop(m_host, processNames, true);
                break;
            default:
                break;
            }
        } catch (XmlRpcRemoteException e) {
            throw new UserException(e);
        }
    }

    public void restartOnEvent(Collection services, Class eventClass) {
        m_eventsToServices.addServices(services, eventClass);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        Collection<SipxService> services = m_eventsToServices.getServices(event.getClass());
        if (!services.isEmpty()) {
            // do not call if set is empty - it's harmless but it triggers topology.xml parsing
            manageServices(services, Command.RESTART);
        }
    }

    static final class EventsToServices<E> {
        private final Map<Class, Set<E>> m_map;

        public EventsToServices() {
            Factory setFactory = new Factory() {
                public Object create() {
                    return new HashSet();
                }
            };
            m_map = MapUtils.lazyMap(new HashMap(), setFactory);
        }

        public void addServices(Collection< ? extends E> services, Class eventClass) {
            Set<E> serviceSet = m_map.get(eventClass);
            serviceSet.addAll(services);
        }

        public Collection<E> getServices(Class eventClass) {
            Set<E> services = new HashSet<E>();
            for (Map.Entry<Class, Set<E>> entry : m_map.entrySet()) {
                Class klass = entry.getKey();
                Collection<E> servicesForKlass = entry.getValue();
                if (klass.isAssignableFrom(eventClass)) {
                    services.addAll(servicesForKlass);
                }
            }
            // do that again this time removing collected services...
            for (Collection<E> servicesForKlass : m_map.values()) {
                servicesForKlass.removeAll(services);
            }
            return services;
        }
    }

    public void enforceRole(Location location) {
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
        manageServices(location, toBeStopped, Command.STOP);
        manageServices(location, toBeStarted, Command.START);
    }
}
