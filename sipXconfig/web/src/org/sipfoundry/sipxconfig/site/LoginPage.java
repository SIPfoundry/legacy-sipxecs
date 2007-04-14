/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.site.user.FirstUser;

public abstract class LoginPage extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "LoginPage";

    public abstract CoreContext getCoreContext();

    public abstract LoginContext getLoginContext();

    public abstract String getPassword();

    public abstract void setPassword(String password);

    public abstract String getUserName();

    public abstract void setUserName(String userName);

    public abstract UserSession getUserSession();

    public void pageBeginRender(PageEvent event) {

        // If there are no users in the DB, then redirect to the FirstUser page to make one.
        // For most pages, Border takes care of this check, but LoginPage doesn't have a Border.
        int userCount = getCoreContext().getUsersCount();
        if (userCount == 0) {
            throw new PageRedirectException(FirstUser.PAGE);
        }

        // If there is only one user in the DB, then this is probably a brand-new system where
        // we just created the superadmin user. Fill in the username automatically to be
        // helpful to the IT guy, who may be learning about sipXconfig for the first time.
        if (userCount == 1) {
            List users = getCoreContext().loadUsers();
            User user = (User) users.get(0);
            setUserName(user.getUserName());
        }
    }

    public String login(IRequestCycle cycle) {
        // always clean password property - use local variable in this function
        String password = getPassword();
        setPassword(null);

        if (!TapestryUtils.isValid(this)) {
            return null;
        }

        LoginContext context = getLoginContext();
        User user = context.checkCredentials(getUserName(), password);
        if (user == null) {
            IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
            delegate.record(getMessages().getMessage("message.loginError"),
                    ValidationConstraint.CONSISTENCY);
            return null;
        }

        UserSession userSession = getUserSession();
        userSession.login(user.getId(), context.isAdmin(user), user.isSupervisor());

        // see border component for setting callback
        ICallback callback = getCallback();
        if (callback != null) {
            callback.performCallback(cycle);
            setCallback(null);
            return null;
        }
        
        // Ignore any callback and go to the home page. If the user was redirected to the login
        // page because the session timed out, then after logging in we will have lost all session
        // data. Trying to execute a callback under these circumstances is hazardous -- see
        // XCF-590.
        return Home.PAGE;        
    }
}
