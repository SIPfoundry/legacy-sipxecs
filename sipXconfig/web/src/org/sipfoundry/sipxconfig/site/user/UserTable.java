/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.hivemind.Messages;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;

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

    public abstract CoreContext getCoreContext();

    public abstract Integer getGroupId();
    
    public abstract String getSearchString();
    
    public abstract boolean getSearchMode();

    public IAsset getUserIcon(User user) {
        return user.isAdmin() ? getAdminUserIcon() : getNormalUserIcon();
    }

    public String getUserIconTitle(User user) {
        Messages messages = getMessages();
        return user.isAdmin() ? messages.getMessage("adminUser") : messages.getMessage("normalUser"); 
    }
    
    public IBasicTableModel getTableModel() {
        String searchQuery = getSearchMode() ? getSearchString() : null;
        return new UserTableModel(getCoreContext(), getGroupId(), searchQuery);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
}
