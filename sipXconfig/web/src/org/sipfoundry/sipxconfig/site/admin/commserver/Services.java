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

import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.components.LocalizedTableRendererSource;

public abstract class Services extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "Services";
    private static final String STATUS_COLUMN = "status";

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Collection getServicesToStart();

    public abstract Collection getServicesToStop();

    public abstract Collection getServicesToRestart();

    public abstract Location getServiceLocation();

    public abstract void setServiceLocation(Location location);
    
    public abstract ExpressionEvaluator getExpressionEvaluator();

    public void pageBeginRender(PageEvent event_) {
        Location location = getServiceLocation();
        if (location != null) {
            return;
        }
        Location[] locations = getSipxProcessContext().getLocations();
        if (locations.length > 0) {
            setServiceLocation(locations[0]);
        }
    }

    public ITableColumn getStatusColumn() {        
        ExpressionTableColumn column = new ExpressionTableColumn(STATUS_COLUMN,
                getMessages().getMessage(STATUS_COLUMN), "status.name", true, getExpressionEvaluator());
        ITableRendererSource rendererSource = new LocalizedTableRendererSource(getMessages(),
                STATUS_COLUMN);
        column.setValueRendererSource(rendererSource);
        return column;
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
        if (services != null) {
            getSipxProcessContext().manageServices(getServiceLocation(), services, operation);
        }
    }
}
