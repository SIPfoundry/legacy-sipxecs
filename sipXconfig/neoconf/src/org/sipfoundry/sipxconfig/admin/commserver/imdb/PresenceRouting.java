/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class PresenceRouting extends DataSetGenerator {

    public PresenceRouting() {
    }

    protected DataSet getType() {
        return DataSet.PRESENCE_ROUTING;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domain = getSipDomain();
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generateUserPref(user, items, domain);
            }
        };
        forAllUsersDo(getCoreContext(), closure);
    }

    private void generateUserPref(User user, List<Map<String, String>> items, String domain) {
        ImAccount imAccount = new ImAccount(user);
        Map<String, String> userPref = addItem(items);
        userPref.put("identity", user.getUserName() + "@" + domain);
        userPref.put("vmOnDnd", Boolean.toString(imAccount.isForwardOnDnd()));
    }
}
