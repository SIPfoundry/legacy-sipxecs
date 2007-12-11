/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.ListEditMap;
import org.apache.tapestry.form.PropertySelection;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * EditEmergencyRouting
 */
public abstract class EditEmergencyRouting extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "EditEmergencyRouting";

    public abstract DialPlanContext getDialPlanContext();

    public abstract EmergencyRouting getEmergencyRouting();

    public abstract void setEmergencyRouting(EmergencyRouting emergencyRouting);

    public abstract ListEditMap getExceptionsMap();

    public abstract void setExceptionsMap(ListEditMap map);

    public abstract void setExceptionItem(RoutingException exception);

    public void pageBeginRender(PageEvent event_) {
        EmergencyRouting emergencyRouting = getEmergencyRouting();
        if (emergencyRouting != null) {
            return;
        }
        emergencyRouting = getDialPlanContext().getEmergencyRouting();
        setEmergencyRouting(emergencyRouting);

        Collection exceptions = emergencyRouting.getExceptions();
        ListEditMap map = new ListEditMap();
        for (Iterator i = exceptions.iterator(); i.hasNext();) {
            RoutingException exception = (RoutingException) i.next();
            map.add(exception.getId(), exception);
        }
        setExceptionsMap(map);
    }

    public void commit() {
        if (!isValid()) {
            return;
        }
        DialPlanContext manager = getDialPlanContext();
        manager.storeEmergencyRouting(getEmergencyRouting());
    }

    /**
     * Called by ListEdit component to retrieve exception object associated with a specific id
     */
    public void synchronizeExceptionItem() {
        ListEditMap exceptionsMap = getExceptionsMap();
        RoutingException exception = (RoutingException) exceptionsMap.getValue();

        if (null == exception) {
            TapestryUtils.staleLinkDetected(this);
        } else {
            setExceptionItem(exception);
        }
    }

    public void addException() {
        EmergencyRouting emergencyRouting = getDialPlanContext().getEmergencyRouting();
        emergencyRouting.addException(new RoutingException());
        getDialPlanContext().storeEmergencyRouting(emergencyRouting);
    }

    public void deleteException(Integer id) {
        getDialPlanContext().removeRoutingException(id);
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        PropertySelection component = (PropertySelection) getComponent("gateways");
        if (null == getEmergencyRouting().getDefaultGateway()) {
            delegate.setFormComponent(component);
            delegate.record("Please configure at least one gateway.",
                    ValidationConstraint.CONSISTENCY);

        }
        return !delegate.getHasErrors();
    }
}
