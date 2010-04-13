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
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class UserStatic extends DataSetGenerator {
    public static final String EXTERNAL_MWI = "voicemail/mailbox/external-mwi";

    @Override
    protected DataSet getType() {
        return DataSet.USER_STATIC;
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
        String externalMwi = user.getSettingValue(EXTERNAL_MWI);
        if (externalMwi != null) {
            Map<String, String> item = addItem(items);
            String userName = user.getUserName();
            String identity = userName + "@" + domainName;
            item.put("identity", identity);
            item.put("event", "message-summary");
            item.put("contact", SipUri.format(externalMwi, domainName, false));
            item.put("from_uri", SipUri.format("IVR", domainName, false));
            item.put("to_uri", SipUri.format(userName, domainName, false));
            item.put("callid", "static-mwi-" + identity);
        }
    }
}
