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

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;

public class Credentials extends DataSetGenerator {
    private CallGroupContext m_callGroupContext;

    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    protected void addItems(List<Map<String, String>> items) {
        String domainName = getSipDomain();
        CoreContext coreContext = getCoreContext();
        String realm = coreContext.getAuthorizationRealm();
        List<User> list = coreContext.loadUsers();
        for (User user : list) {
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
        String sipPasswordHash = callGroup.getSipPasswordHash(realm);
        String uri = SipUri.format(null, callGroup.getName(), domainName);
        addCredentialsItem(items, uri, callGroup.getName(), sipPasswordHash, sipPasswordHash, realm);
    }

    private void addSpecialUser(List<Map<String, String>> items, SpecialUserType specialUserType, String domainName,
            String realm) {
        User user = getCoreContext().getSpecialUser(specialUserType);
        if (user != null) {
            addUser(items, user, domainName, realm);
        }
    }

    protected void addUser(List<Map<String, String>> items, User user, String domainName, String realm) {
        String uri = user.getUri(domainName);
        String sipPasswordHash = user.getSipPasswordHash(realm);
        addCredentialsItem(items, uri, user.getUserName(), sipPasswordHash, user.getPintoken(), realm);
    }

    private void addCredentialsItem(List<Map<String, String>> items, String uri, String name, String sipPasswordHash,
            String pintoken, String realm) {
        Map<String, String> item = addItem(items);
        item.put("uri", uri);
        item.put("realm", realm);
        item.put("userid", name);
        item.put("passtoken", sipPasswordHash);
        item.put("pintoken", pintoken);
        item.put("authtype", "DIGEST");
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
