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
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ListBridges extends BasePage {

    public static final String PAGE = "conference/ListBridges";

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectPage(value = EditBridge.PAGE)
    public abstract EditBridge getEditBridgePage();

    public IPage add() {
        EditBridge editBridge = getEditBridgePage();
        editBridge.setBridgeId(null);
        editBridge.setReturnPage(PAGE);
        return editBridge;
    }

    public IPage edit(Integer bridgeId) {
        EditBridge editBridge = getEditBridgePage();
        editBridge.setBridgeId(bridgeId);
        editBridge.setReturnPage(PAGE);
        return editBridge;
    }
}
