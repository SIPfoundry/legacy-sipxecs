/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.im.ImAccount;

import static org.apache.commons.lang.StringUtils.trimToNull;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserTable extends BaseComponent implements PageBeginRenderListener {

    public static final String COMPONENT = "UserTable";

    @Asset("/images/user.png")
    public abstract IAsset getNormalUserIcon();

    @Asset("/images/adminUser.png")
    public abstract IAsset getAdminUserIcon();

    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract User getCurrentUser();

    public abstract void setSelections(SelectMap selected);

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Parameter
    public abstract Integer getGroupId();

    @Parameter
    public abstract String getSearchString();

    @Parameter(defaultValue = "false")
    public abstract boolean getSearchMode();

    @Parameter
    public abstract IActionListener getUserListener();

    public abstract Integer getBranchId();

    public IBasicTableModel getTableModel() {
        String searchQuery = getSearchMode() ? trimToNull(getSearchString()) : null;
        return new UserTableModel(getCoreContext(), getGroupId(), getBranchId(), searchQuery);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }

    public ImAccount getUserImAccount() {
        return new ImAccount(getCurrentUser());
    }

    public IAsset getUserIcon() {
        return getCurrentUser().isAdmin() ? getAdminUserIcon() : getNormalUserIcon();
    }

    public String getUserIconTitle() {
        String key = getCurrentUser().isAdmin() ? "adminUser" : "normalUser";
        return getMessages().getMessage(key);
    }
}
