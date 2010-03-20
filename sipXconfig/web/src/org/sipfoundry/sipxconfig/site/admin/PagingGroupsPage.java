/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.paging.PagingGroup;
import org.sipfoundry.sipxconfig.paging.PagingProvisioningContext;

public abstract class PagingGroupsPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/PagingGroupsPage";

    public static final String NONE = "NONE";

    public static final String TRACE = "TRACE";

    public static final String DEBUG = "DEBUG";

    @InjectObject(value = "spring:pagingContext")
    public abstract PagingContext getPagingContext();

    @InjectObject(value = "spring:pagingProvisioningContext")
    public abstract PagingProvisioningContext getPagingProvisioningContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setGroups(List<PagingGroup> groups);

    public abstract List<PagingGroup> getGroups();

    public abstract void setCurrentRow(PagingGroup group);

    public abstract Collection getSelectedRows();

    public abstract void setPrefix(String prefix);

    public abstract String getPrefix();

    public abstract void setTraceLevel(String traceLevel);

    public abstract String getTraceLevel();

    @Persist
    public abstract boolean isAdvanced();

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
        // load paging prefix
        if (StringUtils.isEmpty(getPrefix())) {
            String prefix = getPagingContext().getPagingPrefix();
            setPrefix(prefix);
        }

        // load sip trace level
        setTraceLevel(getPagingContext().getSipTraceLevel());

        // load paging groups
        List<PagingGroup> groups = getPagingContext().getPagingGroups();
        setGroups(groups);
    }

    public void savePagingServer() {
        String prefix = getPrefix();
        if (StringUtils.isEmpty(prefix)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("error.pagingPrefix")));
            return;
        }

        List<PagingGroup> groups = getGroups();
        if (groups.size() > 0) {
            getPagingContext().setPagingPrefix(prefix);
            getPagingContext().setSipTraceLevel(getTraceLevel());
        }
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        getPagingContext().deletePagingGroupsById(ids);
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    public void restart() {
        getPagingProvisioningContext().deploy();
    }

    public IPropertySelectionModel getTraceLevelModel() {
        return new StringPropertySelectionModel(new String[] {
            NONE, TRACE, DEBUG
        });
    }
}
