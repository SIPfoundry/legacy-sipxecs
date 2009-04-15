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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class SelectUsers extends BasePage {
    public static final String PAGE = "user/SelectUsers";

    public abstract CoreContext getCoreContext();

    public abstract SelectUsersCallback getCallback();

    public abstract void setCallback(SelectUsersCallback callback);

    public abstract void setTitle(String title);

    public abstract void setPrompt(String title);

    public void select(IRequestCycle cycle) {
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        SelectUsersCallback callback = getCallback();
        callback.setIds(selections.getAllSelected());
        callback.performCallback(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        SelectUsersCallback callback = getCallback();
        callback.performCallback(cycle);
    }
}
