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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.search.IdentityToBean;
import org.sipfoundry.sipxconfig.search.SearchManager;
import org.sipfoundry.sipxconfig.search.UserSearchManager;

public abstract class UserSearch extends BaseComponent {

    public static final String COMPONENT = "UserSearch";

    /** this value is also hardcoded in UserSearch.properties to display error message */
    private static final int MAX_RESULT = 100;

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract List getUsers();

    public abstract void setUsers(List users);

    public abstract CoreContext getCoreContext();

    public abstract SearchManager getSearchManager();

    public abstract UserSearchManager getUserSearchManager();

    public abstract boolean getSimpleSearch();

    public abstract String getQuery();

    public void search() {
        List results = null;
        String query = getQuery();

        IdentityToBean identityToBean = new IdentityToBean(
                getCoreContext());
        if (getSimpleSearch() && StringUtils.isNotBlank(query)) {
            results = getSearchManager().search(User.class, query, 0, MAX_RESULT, null, false,
                    identityToBean);
        } else {
            results = getUserSearchManager().search(getUser(), 0, MAX_RESULT, identityToBean);
        }

        // keep original collection, reference has already been given to other
        // components.
        setUsers(new ArrayList(results));
        // record success message
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
        if (delegate instanceof SipxValidationDelegate) {
            SipxValidationDelegate validator = (SipxValidationDelegate) delegate;
            if (results.size() < MAX_RESULT) {
                String msg = getMessages().format("msg.found", new Integer(results.size()));
                validator.recordSuccess(msg);
            } else {
                String msg = getMessages().format("msg.foundTooMany", new Integer(results.size()));
                validator.record(msg, ValidationConstraint.TOO_LARGE);
            }

        }
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (getUsers() == null) {
            setUsers(Collections.EMPTY_LIST);
        }
        if (getUser() == null) {
            setUser(new User());
        }

        super.renderComponent(writer, cycle);
    }
}
