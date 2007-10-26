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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.User;

public class Credentials extends DataSetGenerator {
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
        addSpecialUser(items, SpecialUser.MEDIA_SERVER, domainName, realm);
        addSpecialUser(items, SpecialUser.PARK_SERVER, domainName, realm);
    }

    private void addSpecialUser(Element items, SpecialUser su, String domainName, String realm) {
        User user = getCoreContext().getSpecialUser(su);
        addUser(items, user, domainName, realm);
    }

    protected void addUser(Element items, User user, String domainName, String realm) {
        Element item = addItem(items);
        item.addElement("uri").setText(user.getUri(domainName));
        item.addElement("realm").setText(realm);
        item.addElement("userid").setText(user.getUserName());
        item.addElement("passtoken").setText(user.getSipPasswordHash(realm));
        item.addElement("pintoken").setText(user.getPintoken());
        item.addElement("authtype").setText("DIGEST");
    }
}
