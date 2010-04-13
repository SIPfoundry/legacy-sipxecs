/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SchedulesTable extends BaseComponent {

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    public abstract List getSchedules();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    // set the "changed" parameter to TRUE
    private void markChanged() {
        IBinding changed = getBinding("changed");
        if (changed != null) {
            changed.setObject(Boolean.TRUE);
        }
    }

    public void deleteSchedules() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        ForwardingContext forwardingContext = getForwardingContext();
        forwardingContext.deleteSchedulesById(ids);
        String msg = getMessages().format("msg.success.delete", Integer.toString(ids.size()));
        TapestryUtils.recordSuccess(this, msg);

        markChanged();
    }
}
