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
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ListBridges extends SipxBasePage {

    public static final String PAGE = "conference/ListBridges";

    public static final String CONFIG = "config";

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectPage(value = EditBridge.PAGE)
    public abstract EditBridge getEditBridgePage();

    public IPage edit(Integer bridgeId) {
        return activateEditPage(bridgeId, CONFIG);
    }

    public IPage active(Integer bridgeId) {
        return activateEditPage(bridgeId, "conferences");
    }

    private IPage activateEditPage(Integer bridgeId, String tab) {
        EditBridge page = getEditBridgePage();
        page.setBridgeId(bridgeId);
        if (tab != null) {
            page.setTab(tab);
        }
        page.setReturnPage(PAGE);
        return page;
    }
}
