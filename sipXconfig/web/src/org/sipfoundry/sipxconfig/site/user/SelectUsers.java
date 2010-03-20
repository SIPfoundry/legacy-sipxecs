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
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class SelectUsers extends SipxBasePage {
    public static final String PAGE = "user/SelectUsers";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract SelectUsersCallback getCallback();

    public abstract void setCallback(SelectUsersCallback callback);

    public abstract void setTitle(String title);

    public abstract void setPrompt(String title);

    @InitialValue(value = "true")
    public abstract void setSimpleSearch(boolean simpleSearch);

    @InitialValue(value = "false")
    public abstract void setUseEmailAddress(boolean useEmailAddress);

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
