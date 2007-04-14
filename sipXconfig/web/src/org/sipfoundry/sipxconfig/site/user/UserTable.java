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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class UserTable extends BaseComponent implements PageBeginRenderListener {

    public static final String COMPONENT = "UserTable";

    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public abstract CoreContext getCoreContext();

    public abstract Integer getGroupId();
    
    public abstract String getSearchString();
    
    public abstract boolean getSearchMode();

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
