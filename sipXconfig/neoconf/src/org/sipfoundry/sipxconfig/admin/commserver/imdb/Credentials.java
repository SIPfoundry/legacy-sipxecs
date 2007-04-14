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

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class Credentials extends DataSetGenerator {
    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    protected void addItems(Element items) {
        String domainName = getSipDomain();
        CoreContext coreContext = getCoreContext();
        String realm = coreContext.getAuthorizationRealm();
        List list = coreContext.loadUsers();
        for (Iterator i = list.iterator(); i.hasNext();) {
            User user = (User) i.next();
            Element item = addItem(items);
            addUser(item, user, domainName, realm);
        }
    }

    protected void addUser(Element item, User user, String domainName, String realm) {
        item.addElement("uri").setText(user.getUri(domainName));
        item.addElement("realm").setText(realm);
        item.addElement("userid").setText(user.getUserName());
        item.addElement("passtoken").setText(user.getSipPasswordHash(realm));
        item.addElement("pintoken").setText(user.getPintoken());
        item.addElement("authtype").setText("DIGEST");
    }
}
