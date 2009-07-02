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

import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

public class UserForward extends DataSetGenerator {
    private ForwardingContext m_forwardingContext;

    @Override
    protected DataSet getType() {
        return DataSet.USER_FORWARD;
    }

    @Override
    protected void addItems(List<Map<String, String>> items) {
        String domainName = getSipDomain();
        List<User> list = getCoreContext().loadUsers();
        for (User user : list) {
            addUser(items, user, domainName);
        }
    }

    protected void addUser(List<Map<String, String>> items, User user, String domainName) {
        Map<String, String> item = addItem(items);
        String identity = user.getUserName() + "@" + domainName;
        item.put("identity", identity);
        CallSequence cs = m_forwardingContext.getCallSequenceForUser(user);
        item.put("cfwdtime", Integer.toString(cs.getCfwdTime()));
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }
}
