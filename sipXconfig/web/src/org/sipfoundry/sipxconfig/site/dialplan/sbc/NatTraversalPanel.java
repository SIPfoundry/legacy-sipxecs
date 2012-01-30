/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.nattraversal.NatSettings;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;

@ComponentClass
public abstract class NatTraversalPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:natTraveral")
    public abstract NatTraversal getNatTraversal();

    public abstract NatSettings getSettings();

    public abstract void setSettings(NatSettings settings);

    public void pageBeginRender(PageEvent event_) {
        if (getSettings() == null) {
            setSettings(getNatTraversal().getSettings());
        }
    }

    public void apply() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getNatTraversal().saveSettings(getSettings());
        TapestryUtils.recordSuccess(this, getMessages().getMessage("msg.actionSuccess"));
    }
}
