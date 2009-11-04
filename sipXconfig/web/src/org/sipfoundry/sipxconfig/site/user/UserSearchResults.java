/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.User;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserSearchResults extends UserTable {

    @Parameter(required = true)
    public abstract List<User> getUsers();

    @Parameter(defaultValue = "false")
    public abstract boolean getUseEmailAddress();

    public String getColumns() {
        if (getUseEmailAddress()) {
            return "* userName,lastName,firstName,!emailAddress";
        }
        return "* userName,lastName,firstName,aliases:aliasesString";
    }
}
