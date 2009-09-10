/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class UserLocation extends DataSetGenerator {

    @Override
    protected DataSet getType() {
        return DataSet.USER_LOCATION;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                addUser(items, user, domainName);
            }
        };
        forAllUsersDo(getCoreContext(), closure);
    }

    protected void addUser(List<Map<String, String>> items, User user, String domainName) {
        String url = user.getAddrSpec(domainName);
        Branch site = user.getSite();
        if (site != null) {
            addUserLocationItem(items, url, site.getName());
        }
    }

    private void addUserLocationItem(List<Map<String, String>> items, String url, String siteName) {
        Map<String, String> item = addItem(items);
        item.put("identity", url);
        item.put("location", siteName);
    }
}
