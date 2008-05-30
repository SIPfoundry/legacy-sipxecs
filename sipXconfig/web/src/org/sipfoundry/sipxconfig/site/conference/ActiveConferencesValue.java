/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ActiveConferencesValue extends BaseComponent {

    @Parameter(required = true)
    public abstract void setActiveConferenceCount(Integer activeConferenceCount);

    @Parameter(required = true)
    public abstract void setBridge(Bridge bridge);

    @Asset("/images/go.png")
    public abstract IAsset getGoIcon();

    public IPage activeConferences(IRequestCycle cycle) {
        Integer bridgeId = TapestryUtils.getBeanId(cycle);
        ListActiveConferences activePage = (ListActiveConferences) cycle.getPage(ListActiveConferences.PAGE);
        activePage.setBridgeId(bridgeId);
        return activePage;
    }
}
