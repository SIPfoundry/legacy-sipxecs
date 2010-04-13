/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.service.EditPresenceService;

public abstract class ListAcdServers extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "acd/ListAcdServers";

    public abstract AcdContext getAcdContext();

    public abstract Collection getServers();

    public abstract void setServers(Collection servers);

    public void pageBeginRender(PageEvent event_) {
        if (getServers() == null) {
            setServers(getAcdContext().getServers());
        }
    }

    public IPage editServer(IRequestCycle cycle, Integer id) {
        AcdServerPage editPage = (AcdServerPage) cycle.getPage(AcdServerPage.PAGE);
        editPage.setAcdServerId(id);
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage activatePresenceServerPage(IRequestCycle cycle) {
        EditPresenceService page = (EditPresenceService) cycle.getPage(EditPresenceService.PAGE);
        page.setReturnPage(ListAcdServers.PAGE);
        return page;
    }

    public void formSubmit() {
        if (TapestryUtils.isValid(this)) {
            setServers(null);
        }
    }
}
