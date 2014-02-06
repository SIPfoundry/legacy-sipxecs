/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.paging;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.paging.PagingFeatureContext;
import org.sipfoundry.sipxconfig.paging.PagingGroup;
import org.sipfoundry.sipxconfig.paging.PagingSettings;

public abstract class PagingGroupsPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "paging/PagingGroupsPage";

    @InjectObject(value = "spring:pagingContext")
    public abstract PagingContext getPagingContext();

    @InjectObject(value = "spring:pagingFeatureContext")
    public abstract PagingFeatureContext getPagingFeatureContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setGroups(List<PagingGroup> groups);

    public abstract List<PagingGroup> getGroups();

    public abstract void setCurrentRow(PagingGroup group);

    public abstract Collection getSelectedRows();

    public abstract PagingSettings getSettings();

    public abstract void setSettings(PagingSettings settings);

    @Persist
    @InitialValue(value = "literal:paging")
    public abstract String getTab();

    public IPage addPagingGroup(IRequestCycle cycle) {
        EditPagingGroupPage page = (EditPagingGroupPage) cycle.getPage(EditPagingGroupPage.PAGE);
        page.addPagingGroup(getPage().getPageName());
        return page;
    }

    public IPage editPagingGroup(IRequestCycle cycle, Integer groupId) {
        EditPagingGroupPage page = (EditPagingGroupPage) cycle.getPage(EditPagingGroupPage.PAGE);
        page.editPagingGroup(groupId, getPage().getPageName());
        return page;
    }

    public void pageBeginRender(PageEvent event) {
        if (getSettings() == null) {
            setSettings(getPagingContext().getSettings());
        }

        // load paging groups
        List<PagingGroup> groups = getPagingContext().getPagingGroups();
        setGroups(groups);
    }

    public void apply(IRequestCycle cycle) {
        getPagingContext().saveSettings(getSettings());
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        getPagingFeatureContext().deletePagingGroupsById(ids);
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }
}
