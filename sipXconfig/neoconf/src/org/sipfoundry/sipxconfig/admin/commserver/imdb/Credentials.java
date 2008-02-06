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

import org.dom4j.Element;
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

    protected void addItems(Element items) {
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

    void addCallGroup(Element items, CallGroup callGroup, String domainName, String realm) {
        String sipPasswordHash = callGroup.getSipPasswordHash(realm);
        String uri = SipUri.format(null, callGroup.getName(), domainName);
        addCredentialsItem(items, uri, callGroup.getName(), sipPasswordHash, sipPasswordHash,
                realm);
    }

    private void addSpecialUser(Element items, SpecialUserType specialUserType,
            String domainName, String realm) {
        User user = getCoreContext().getSpecialUser(specialUserType);
        if (user != null) {
            addUser(items, user, domainName, realm);
        }
    }

    protected void addUser(Element items, User user, String domainName, String realm) {
        String uri = user.getUri(domainName);
        String sipPasswordHash = user.getSipPasswordHash(realm);
        addCredentialsItem(items, uri, user.getUserName(), sipPasswordHash, user.getPintoken(),
                realm);
    }

    private void addCredentialsItem(Element items, String uri, String name,
            String sipPasswordHash, String pintoken, String realm) {
        Element item = addItem(items);
        item.addElement("uri").setText(uri);
        item.addElement("realm").setText(realm);
        item.addElement("userid").setText(name);
        item.addElement("passtoken").setText(sipPasswordHash);
        item.addElement("pintoken").setText(pintoken);
        item.addElement("authtype").setText("DIGEST");
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
