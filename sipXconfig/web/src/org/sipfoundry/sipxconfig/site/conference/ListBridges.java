/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ListBridges extends BasePage {

    public static final String PAGE = "ListBridges";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public void add(IRequestCycle cycle) {
        EditBridge editBridge = (EditBridge) cycle.getPage(EditBridge.PAGE);
        editBridge.setBridgeId(null);
        editBridge.setReturnPage(PAGE);
        cycle.activate(editBridge);
    }

    public IPage edit(IRequestCycle cycle) {
        Integer bridgeId = TapestryUtils.getBeanId(cycle);
        EditBridge editBridge = (EditBridge) cycle.getPage(EditBridge.PAGE);
        editBridge.setBridgeId(bridgeId);
        editBridge.setReturnPage(PAGE);
        return editBridge;
    }
}
