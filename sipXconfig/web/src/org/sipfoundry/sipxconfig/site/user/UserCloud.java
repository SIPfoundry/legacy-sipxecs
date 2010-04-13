/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserCloud extends BaseComponent {
    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(value = EditUser.PAGE)
    public abstract EditUser getEditUserPage();

    @InjectPage(value = NewUser.PAGE)
    public abstract NewUser getNewUserPage();

    public abstract List getUsers();

    public abstract void setUsers(List users);

    public abstract int getCount();

    public abstract void setCount(int count);

    public abstract User getUser();

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getUsers() == null) {
            CoreContext coreContext = getCoreContext();
            // TODO: replace by loading lightweight user substitute
            List<User> users = coreContext.loadUsersByPage(null, null, null, 0, 25, "userName", true);
            setUsers(users);
            setCount(coreContext.getUsersCount());
        }
    }

    public IPage edit(Integer userId) {
        EditUser page = getEditUserPage();
        page.setUserId(userId);
        page.setReturnPage(getPage());
        return page;
    }

    public IPage add() {
        NewUser page = getNewUserPage();
        page.setReturnPage(getPage());
        return page;
    }

    public String getTitle() {
        return getMessages().format("title", getCount());
    }
}
