/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static java.util.Collections.sort;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.RestartListener;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDescriptor;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxCallResolverService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.site.acd.AcdServerPage;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;
import org.sipfoundry.sipxconfig.site.sbc.EditSbcDevice;
import org.sipfoundry.sipxconfig.site.service.EditCallResolverService;
import org.sipfoundry.sipxconfig.site.service.EditPresenceService;
import org.sipfoundry.sipxconfig.site.service.EditSipxService;

import static org.apache.commons.lang.StringUtils.join;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Undefined;
import static org.sipfoundry.sipxconfig.components.LocalizationUtils.getMessage;

public abstract class ServicesTable extends BaseComponent {
    public static final Map<String, String> SERVICE_MAP = new HashMap<String, String>();
    static {
        SERVICE_MAP.put(SipxPresenceService.BEAN_ID, EditPresenceService.PAGE);
        SERVICE_MAP.put(SipxCallResolverService.BEAN_ID, EditCallResolverService.PAGE);
        SERVICE_MAP.put(SipxBridgeService.BEAN_ID, EditSbcDevice.PAGE);
        SERVICE_MAP.put(SipxAcdService.BEAN_ID, AcdServerPage.PAGE);
        //There is currently no service related editable parameters for acc code
        //other than the extension (in Authorization Code screen).
        //When more service related editable parameters are added, uncomment below
        //or set editable property in service.xml
        //SERVICE_MAP.put(SipxAccCodeService.BEAN_ID, EditAccCodeService.PAGE);
    }

    @InjectObject("service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @InjectObject("spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    @InjectObject("spring:acdContext")
    public abstract AcdContext getAcdContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject("spring:sipXbridgeSbcModel")
    public abstract SbcDescriptor getSbcDescriptor();

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectPage(EditLocationPage.PAGE)
    public abstract EditLocationPage getEditLocationPage();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    @InjectObject("spring:restartListener")
    public abstract RestartListener getRestartListener();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract ServerStatusSqueezeAdapter getServerStatusConverter();

    @Bean(initializer = "array=sipxProcessContext.locations,labelExpression=id")
    public abstract ObjectSelectionModel getLocationModel();

    public abstract ServiceStatus getCurrentRow();

    public abstract int getCurrentRowId();

    @Parameter(required = true)
    public abstract Location getServiceLocation();

    public abstract void setServiceLocation(Location location);

    @Parameter(required = true)
    public abstract List<BreadCrumb> getBreadCrumbs();

    public abstract void setBreadCrumbs(List<BreadCrumb> breadCrumbs);

    public abstract void setServiceStatusCached(Object[] serviceStatus);

    public abstract Object[] getServiceStatusCached();

    @Asset("/images/cog.png")
    public abstract IAsset getServiceIcon();

    @Asset("/images/service_restart.png")
    public abstract IAsset getRestartIcon();

    private void initializeDefaultEditServices() {
        Collection<SipxService> services = getSipxServiceManager().getServiceDefinitions();
        for (SipxService service : services) {
            if (service.isEditable()) {
                SERVICE_MAP.put(service.getBeanId(), EditSipxService.PAGE);
            }
        }
    }

    public String getServiceLabel() {
        String serviceBeanId = getCurrentRow().getServiceBeanId();
        String key = "label." + serviceBeanId;
        return getMessage(getMessages(), key, serviceBeanId);
    }

    public String getServiceRoles() {
        String beanId = getCurrentRow().getServiceBeanId();
        SipxService service = getSipxServiceManager().getServiceByBeanId(beanId);
        List<SipxServiceBundle> serviceBundles = service.getBundles(getServiceLocation());
        List<String> rolesLabels = new ArrayList<String>();
        for (SipxServiceBundle bundle : serviceBundles) {
            String serviceBundleName = bundle.getName();
            rolesLabels.add(getMessage(getMessages(), "bundle." + serviceBundleName, serviceBundleName));
        }
        sort(rolesLabels);
        return join(rolesLabels.iterator(), ", ");
    }

    public boolean isNeedsRestart() {
        return getCurrentRow().isNeedsRestart();
    }

    public boolean isNeedsReload() {
        return getCurrentRow().isNeedsReload();
    }

    public Object[] getServiceStatus() {
        initializeDefaultEditServices();
        Object[] serviceStatus = getServiceStatusCached();
        if (serviceStatus == null) {
            serviceStatus = retrieveServiceStatus(getServiceLocation());
            setServiceStatusCached(serviceStatus);
        }
        return serviceStatus;
    }

    public Object[] retrieveServiceStatus(Location location) {
        if (location == null || location.getServices() == null) {
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
        try {
            return getSipxProcessContext().getStatus(location, true);
        } catch (UserException e) {
            // Hide user exception -
            // Services status is Undefined when we cannot get real status from server
            // This is due to XML-RPC call failure
            Collection<ServiceStatus> serviceStatusList = new ArrayList<ServiceStatus>();
            for (LocationSpecificService lss : location.getServices()) {
                SipxService service = lss.getSipxService();
                String serviceName = service.getProcessName();
                if (serviceName != null) {
                    boolean needsRestart = getSipxProcessContext().needsRestart(location, service);
                    boolean needsReload = getSipxProcessContext().needsReload(location, service);
                    ServiceStatus status = new ServiceStatus(service.getBeanId(), Undefined, needsRestart, needsReload);
                    serviceStatusList.add(status);
                }
            }
            return serviceStatusList.toArray();
        }
    }

    public boolean getCurrentRowHasServiceLink() {
        return SERVICE_MAP.containsKey(getCurrentRow().getServiceBeanId());
    }

    public IPage editService(IRequestCycle cycle, String serviceBeanId, Integer locationId) {
        PageWithCallback page = (PageWithCallback) cycle.getPage(SERVICE_MAP.get(serviceBeanId));
        if (page instanceof EditSbcDevice) {
            Location location = getLocationsManager().getLocation(locationId);
            BridgeSbc bridgeSbc = getSbcDeviceManager().getBridgeSbc(location);
            if (null != bridgeSbc) {
                return EditSbcDevice.getEditPage(cycle, bridgeSbc.getId(), getEditLocationPage().getPage());
            }
            return EditSbcDevice.getAddPage(cycle, getSbcDescriptor(), getEditLocationPage().getPage());
        }
        if (page instanceof AcdServerPage) {
            AcdServer acdServer = getAcdContext().getAcdServerForLocationId(locationId);
            if (acdServer != null) {
                ((AcdServerPage) page).setAcdServerId(acdServer.getId());
            }
        } else if (page instanceof EditSipxService) {
            EditSipxService servicePage = (EditSipxService) page;
            servicePage.setBeanId(serviceBeanId);
            page.setReturnPage(getEditLocationPage().getPage(), getBreadCrumbs());
        }
        return page;
    }

    /**
     * Registered as form listener - forces service status update every time form is submitted
     */
    public void refresh() {
        setServiceStatusCached(null);
    }

    public void removeService() {
        manageServices(SipxProcessContext.Command.STOP);
        Collection<String> selectedBeanIds = getSelections().getAllSelected();
        for (String beanId : selectedBeanIds) {
            getServiceLocation().removeServiceByBeanId(beanId);
        }
        getLocationsManager().storeLocation(getServiceLocation());
    }

    public void start() {
        manageServices(SipxProcessContext.Command.START);
    }

    public void stop() {
        manageServices(SipxProcessContext.Command.STOP);
    }

    public IPage restart() {
        return manageServices(SipxProcessContext.Command.RESTART);
    }

    public IPage reload() {
        return manageServices(SipxProcessContext.Command.RELOAD);
    }

    private IPage manageServices(SipxProcessContext.Command operation) {
        Collection<String> serviceBeanIds = getSelections().getAllSelected();
        if (serviceBeanIds == null) {
            return null;
        }

        SipxServiceManager sipxServiceManager = getSipxServiceManager();
        List<SipxService> services = new ArrayList<SipxService>(serviceBeanIds.size());

        boolean config = false;
        boolean restartOperation = operation.equals(Command.RESTART);

        for (String beanId : serviceBeanIds) {
            if (StringUtils.equals(beanId, SipxConfigService.BEAN_ID)) {
                config = true;
            }
            SipxService service = sipxServiceManager.getServiceByBeanId(beanId);
            if (service != null && (!config || restartOperation)) {
                services.add(service);
            }
        }

        if (config && restartOperation) {
            return performRestart(services);
        } else {
            getSipxProcessContext().manageServices(getServiceLocation(), services, operation);
            refresh();
            if (config && !restartOperation) {
                throw new UserException("&only.restart.available");
            }
            return null;
        }
    }

    private IPage performRestart(List<SipxService> services) {
        RestartListener restartListener = getRestartListener();
        Map<Location, List<SipxService>> servicesMap = new HashMap<Location, List<SipxService>>();
        servicesMap.put(getServiceLocation(), services);
        restartListener.setServicesMap(servicesMap);
        WaitingPage waitingPage = getWaitingPage();
        waitingPage.setWaitingListener(restartListener);
        return waitingPage;
    }
}
