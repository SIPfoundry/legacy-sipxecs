/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Collection;

import org.apache.commons.lang.ArrayUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.DomainConfigReplicatedEvent;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public abstract class Services extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "Services";

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject(value = "spring:domainManager")
    public abstract DomainManager getDomainManager();

    @Bean
    public abstract SelectMap getSelections();

    public abstract Location getServiceLocation();

    public abstract void setServiceLocation(Location location);

    public abstract void setServiceStatus(Object[] serviceStatus);

    public abstract Object[] getServiceStatus();

    public void pageBeginRender(PageEvent event_) {
        Location location = getServiceLocation();
        if (location == null) {
            Location[] locations = getSipxProcessContext().getLocations();
            if (locations.length > 0) {
                location = locations[0];
                setServiceLocation(location);
            }
        }

        Object[] serviceStatus = getServiceStatus();
        if (serviceStatus == null) {
            serviceStatus = retrieveServiceStatus(location);
            setServiceStatus(serviceStatus);
        }
    }

    public Object[] retrieveServiceStatus(Location location) {
        if (location == null) {
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
        try {
            return getSipxProcessContext().getStatus(location);
        } catch (UserException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record(new ValidatorException(e.getMessage()));
            return ArrayUtils.EMPTY_OBJECT_ARRAY;
        }
    }

    /**
     * Registered as form listener - forces service status update every time form is submitted
     */
    public void refresh() {
        setServiceStatus(null);
    }

    public void start() {
        manageServices(SipxProcessContext.Command.START);
    }

    public void stop() {
        manageServices(SipxProcessContext.Command.STOP);
    }

    public void restart() {
        manageServices(SipxProcessContext.Command.RESTART);
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
                getSipxProcessContext().restartOnEvent(services, DomainConfigReplicatedEvent.class);
            } else {
                getSipxProcessContext().manageServices(getServiceLocation(), services, operation);
            }
        } catch (UserException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record(new ValidatorException(e.getMessage()));
        }
    }
}
