/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.ArrayUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.DomainConfigReplicatedEvent;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.site.service.EditParkService;
import org.sipfoundry.sipxconfig.site.service.EditPresenceService;
import org.sipfoundry.sipxconfig.site.service.EditProxyService;
import org.sipfoundry.sipxconfig.site.service.EditRegistrarService;

public abstract class ServicesTable extends BaseComponent implements PageBeginRenderListener {

    public static final Map<String, String> SERVICE_MAP = new HashMap<String, String>();
    static {
        SERVICE_MAP.put("SIPXProxy", EditProxyService.PAGE);
        SERVICE_MAP.put("SIPRegistrar", EditRegistrarService.PAGE);
        SERVICE_MAP.put("ParkServer", EditParkService.PAGE);
        SERVICE_MAP.put("PresenceServer", EditPresenceService.PAGE);
    }

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject(value = "spring:domainManager")
    public abstract DomainManager getDomainManager();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract ServerStatusSqueezeAdapter getServerStatusConverter();

    @Bean(initializer = "array=sipxProcessContext.locations,labelExpression=id")
    public abstract ObjectSelectionModel getLocationModel();

    public abstract ServiceStatus getCurrentRow();

    @Parameter(required = true)
    public abstract Location getServiceLocation();

    public abstract void setServiceLocation(Location location);

    public abstract void setServiceStatus(Object[] serviceStatus);

    public abstract Object[] getServiceStatus();

    public void pageBeginRender(PageEvent event_) {
        Object[] serviceStatus = getServiceStatus();
        if (serviceStatus == null) {
            serviceStatus = retrieveServiceStatus(getServiceLocation());
            setServiceStatus(serviceStatus);
        }
    }

    public Object[] retrieveServiceStatus(Location location) {
        if (location == null || location.getSipxServices() == null) {
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
        try {
            return getSipxProcessContext().getStatus(location, true);
            
        } catch (UserException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record(new ValidatorException(e.getMessage()));
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
    }

    public boolean getCurrentRowHasServiceLink() {
        return SERVICE_MAP.containsKey(getCurrentRow().getServiceName());
    }

    public IPage editService(IRequestCycle cycle, String serviceName) {
        PageWithCallback page = (PageWithCallback) cycle.getPage(SERVICE_MAP.get(serviceName));
        page.setReturnPage(EditLocationPage.PAGE);
        return page;
    }

    /**
     * Registered as form listener - forces service status update every time form is submitted
     */
    public void refresh() {
        setServiceStatus(null);
    }

    public void start() {
        manageServices(SipxProcessContext.Command.START);
        refresh();
    }

    public void stop() {
        manageServices(SipxProcessContext.Command.STOP);
        refresh();
    }

    public void restart() {
        manageServices(SipxProcessContext.Command.RESTART);
        refresh();
    }

    private void manageServices(SipxProcessContext.Command operation) {
        Collection services = getSelections().getAllSelected();
        if (services == null) {
            return;
        }
        try {
            // HACK: this is ugly. we should refactor process context to handle all management
            // commands when it receives an event.
            if (Command.RESTART.equals(operation)) {
                getDomainManager().replicateDomainConfig();
                getSipxProcessContext().restartOnEvent(services,
                        DomainConfigReplicatedEvent.class);
            } else {
                getSipxProcessContext().manageServices(getServiceLocation(), services, operation);
            }
        } catch (UserException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record(new ValidatorException(e.getMessage()));
        }
    }
}
