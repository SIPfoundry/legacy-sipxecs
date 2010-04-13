/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class Credentials extends DataSetGenerator {
    private CallGroupContext m_callGroupContext;

    @Override
    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        CoreContext coreContext = getCoreContext();
        final String realm = coreContext.getAuthorizationRealm();

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                addUser(items, user, domainName, realm);
            }
        };
        forAllUsersDo(getCoreContext(), closure);

        List<InternalUser> internalUsers = getCoreContext().loadInternalUsers();
        for (InternalUser user : internalUsers) {
            addUser(items, user, domainName, realm);
        }

        for (SpecialUserType specialUserType : SpecialUserType.values()) {
            addSpecialUser(items, specialUserType, domainName, realm);
        }

        List<CallGroup> callGroups = m_callGroupContext.getCallGroups();
        for (CallGroup callGroup : callGroups) {
            if (callGroup.isEnabled()) {
                addCallGroup(items, callGroup, domainName, realm);
            }
        }
    }

    void addCallGroup(List<Map<String, String>> items, CallGroup callGroup, String domainName, String realm) {
        String uri = SipUri.format(null, callGroup.getName(), domainName);
        String sipPassword = callGroup.getSipPassword();
        String sipPasswordHash = callGroup.getSipPasswordHash(realm);
        addCredentialsItem(items, uri, callGroup.getName(), sipPassword, sipPasswordHash, realm);
    }

    private void addSpecialUser(List<Map<String, String>> items, SpecialUserType specialUserType, String domainName,
            String realm) {
        User user = getCoreContext().getSpecialUser(specialUserType);
        if (user != null) {
            addUser(items, user, domainName, realm);
        }
    }

    protected void addUser(List<Map<String, String>> items, AbstractUser user, String domainName, String realm) {
        String uri = user.getUri(domainName);
        addCredentialsItem(items, uri, user.getUserName(), user.getSipPassword(), user.getPintoken(), realm);
    }

    private void addCredentialsItem(List<Map<String, String>> items, String uri, String name, String sipPassword,
            String pintoken, String realm) {
        Map<String, String> item = addItem(items);
        item.put("uri", uri);
        item.put("realm", realm);
        item.put("userid", name);
        item.put("passtoken", defaultString(sipPassword));
        item.put("pintoken", pintoken);
        item.put("authtype", "DIGEST");
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
