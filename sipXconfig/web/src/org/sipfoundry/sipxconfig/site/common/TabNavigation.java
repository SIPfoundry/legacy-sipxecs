/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class TabNavigation extends BaseComponent {

    public abstract String getCurrentTab();

    @Parameter(required = false, defaultValue = "literal:tabNavigation")
    public abstract String getElementId();

    @Parameter(required = true)
    public abstract Collection getTabNames();

    @Parameter()
    public abstract void setSelectedTab(String section);

    @Parameter(required = false, defaultValue = "false")
    public abstract boolean isSubmit();

    @Parameter(required = false, defaultValue = "true")
    public abstract void setRenderCondition(boolean renderCondition);

    @Parameter(required = false, defaultValue = "true")
    public abstract void setUseLeftNavigation(boolean useLeftNavigation);

    public void activateTab(String tabName) {
        setSelectedTab(tabName);
    }

    public void submitAndActivateTab(String tabName) {
        if (TapestryUtils.isValid(this)) {
            setSelectedTab(tabName);
        }
    }

    public String getCurrentTabLabel() {
        return getContainer().getMessages().getMessage("tab." + getCurrentTab());
    }
}
