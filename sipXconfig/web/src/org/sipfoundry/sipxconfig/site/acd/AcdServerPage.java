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

import java.io.Serializable;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatistics;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AcdServerPage extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "acd/AcdServerPage";

    public abstract AcdContext getAcdContext();

    public abstract Serializable getAcdServerId();

    public abstract void setAcdServerId(Serializable id);

    public abstract AcdServer getAcdServer();

    public abstract void setAcdServer(AcdServer acdServer);

    public abstract boolean getChanged();

    public abstract void setTab(String tab);

    public abstract AcdStatistics getAcdStatistics();

    public abstract void setAcdStatistics(AcdStatistics stats);

    public void pageBeginRender(PageEvent event_) {
        if (getAcdServer() != null) {
            return;
        }
        if (getAcdServerId() != null) {
            AcdServer server = getAcdContext().loadServer(getAcdServerId());
            setAcdServer(server);
        } else {
            AcdServer server = getAcdContext().newServer();
            setAcdServer(server);
            // make sure that config tab is available when server is new
            setTab("config");
        }
        if (getAcdStatistics() == null) {
            setAcdStatistics(new AcdStatisticsImpl(getAcdContext()));
        }
    }

    public void apply() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        AcdServer acdServer = getAcdServer();
        getAcdContext().store(acdServer);
        Integer id = acdServer.getId();
        setAcdServerId(id);
    }

    public void formSubmit() {
        if (getChanged()) {
            setAcdServer(null);
        }
    }

    public IPage addLine(IRequestCycle cycle) {
        EditAcdLine editPage = (EditAcdLine) cycle.getPage(EditAcdLine.PAGE);
        editPage.setAcdLineId(null);
        editPage.setAcdServerId(getAcdServerId());
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage editLine(IRequestCycle cycle, Integer id) {
        EditAcdLine editPage = (EditAcdLine) cycle.getPage(EditAcdLine.PAGE);
        editPage.setAcdLineId(id);
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage addQueue(IRequestCycle cycle) {
        EditAcdQueue editPage = (EditAcdQueue) cycle.getPage(EditAcdQueue.PAGE);
        editPage.setAcdQueueId(null);
        editPage.setAcdServerId(getAcdServerId());
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage editQueue(IRequestCycle cycle, Integer id) {
        EditAcdQueue editPage = (EditAcdQueue) cycle.getPage(EditAcdQueue.PAGE);
        editPage.setAcdQueueId(id);
        editPage.setReturnPage(this);
        return editPage;
    }

    @Override
    public String getBreadCrumbTitle() {
        return "&crumb.editSipxService";
    }
}
