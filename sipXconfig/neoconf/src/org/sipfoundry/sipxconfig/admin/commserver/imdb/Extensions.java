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

import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class Extensions extends DataSetGenerator {

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                addItem(items, domainName, user);
            }
        };
        forAllUsersDo(getCoreContext(), closure);
    }

    /**
     * Add shortest numeric alias as extension
     *
     * @param items
     * @param domainName
     * @param user
     */
    private void addItem(List<Map<String, String>> items, String domainName, User user) {
        if (user.hasNumericUsername()) {
            // no need add those users
            return;
        }
        String extension = user.getExtension(false);
        // only generate an empty if extension exist and extension is different from user name
        if (extension != null) {
            Map<String, String> item = addItem(items);

            item.put("uri", user.getUri(domainName));
            // add first found numeric alias
            item.put("extension", extension);
        }
    }

    @Override
    protected DataSet getType() {
        return DataSet.EXTENSION;
    }
}
