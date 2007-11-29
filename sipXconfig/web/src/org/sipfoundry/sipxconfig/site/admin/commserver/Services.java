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
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class Services extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "Services";

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Collection getServicesToStart();

    public abstract Collection getServicesToStop();

    public abstract Collection getServicesToRestart();

    public abstract Location getServiceLocation();

    public abstract void setServiceLocation(Location location);

    public abstract ExpressionEvaluator getExpressionEvaluator();

    public abstract void setServiceStatus(Object[] serviceStatus);

    public abstract Object[] getServiceStatus();

    public void pageBeginRender(PageEvent event_) {
        Object[] serviceStatus = getServiceStatus();
        if (serviceStatus != null) {
            return;
        }

        Location location = getServiceLocation();
        if (location == null) {
            Location[] locations = getSipxProcessContext().getLocations();
            if (locations.length > 0) {
                location = locations[0];
                setServiceLocation(location);
            }
        }

        serviceStatus = retrieveServiceStatus(location);
        setServiceStatus(serviceStatus);
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

    public void formSubmit() {
        // Ideally the start/stop/restart operations would be implemented in button listeners.
        // However, Tapestry 3.0 has a bug in it such that when a component listener is
        // triggered, data is available only for those components that precede it in the
        // rendering order. So wait until formSubmit, at which time all data will be there.

        manageServices(getServicesToStart(), SipxProcessContext.Command.START);
        manageServices(getServicesToStop(), SipxProcessContext.Command.STOP);
        manageServices(getServicesToRestart(), SipxProcessContext.Command.RESTART);
    }

    private void manageServices(Collection services, SipxProcessContext.Command operation) {
        if (services == null) {
            return;
        }
        try {
            getSipxProcessContext().manageServices(getServiceLocation(), services, operation);
        } catch (UserException e) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record(new ValidatorException(e.getMessage()));
        }
    }
}
