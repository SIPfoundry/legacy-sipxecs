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


import static org.sipfoundry.sipxconfig.components.LocalizationUtils.getMessage;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;
import org.sipfoundry.sipxconfig.site.common.BreadCrumb;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public abstract class ServicesTable extends BaseComponent {
    public static final Map<String, String> SERVICE_MAP = new HashMap<String, String>();
    public static final Log LOG = LogFactory.getLog(ServicesTable.class);

    private static final String LABEL = "label.";

    @InjectObject("service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:snmpManager")
    public abstract SnmpManager getSnmpManager();

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

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


    public String getServiceLabel() {
        String serviceBeanId = getCurrentRow().getServiceBeanId();
        String key = LABEL + serviceBeanId;
        return getMessage(getMessages(), key, serviceBeanId);
    }

    public Object[] getServiceStatus() {
        Object[] serviceStatus = getServiceStatusCached();
        if (serviceStatus == null) {
            serviceStatus = retrieveServiceStatus(getServiceLocation());
            setServiceStatusCached(serviceStatus);
        }

        Map<String, Object> sortedMap = new TreeMap<String, Object>();
        for (Object obj : serviceStatus) {
            String label = getMessages().getMessage(LABEL + ((ServiceStatus) obj).getServiceBeanId());
            sortedMap.put(label.toLowerCase(), obj);
        }

        return sortedMap.values().toArray();
    }

    public Object[] retrieveServiceStatus(Location location) {
        if (location == null) {
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
        List<ServiceStatus> statuses = new ArrayList<ServiceStatus>();
        try {
            statuses = getSnmpManager().getServicesStatuses(location);
        } catch (UserException ex) {
            LOG.error(ex.getMessage());
        }
        if (statuses == null || statuses.size() == 0) {
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
        return statuses.toArray();
    }

    /**
     * Registered as form listener - forces service status update every time form is submitted
     */
    public void refresh() {
        setServiceStatusCached(null);
    }

    public void restart() {
        @SuppressWarnings("unchecked")
        Collection<String> selected = getSelections().getAllSelected();
        if (selected.size() > 0) {
            List<ProcessDefinition> defs = getSnmpManager().getProcessDefinitions(getServiceLocation(), selected);
            getSnmpManager().restartProcesses(getServiceLocation(), defs);
            String msg = getMessages().getMessage("statusMessage.servicesRestarted");
            ((SipxValidationDelegate) TapestryUtils.getValidator(getPage())).recordSuccess(msg);
        }
    }
}
