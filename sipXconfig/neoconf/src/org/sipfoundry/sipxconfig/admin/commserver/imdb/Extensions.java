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
import org.sipfoundry.sipxconfig.common.User;

public class Extensions extends DataSetGenerator {

    protected void addItems(Element items) {
        String domainName = getSipDomain();
        List<User> users = getCoreContext().loadUsers();
        for (User user : users) {
            addItem(items, domainName, user);
        }
    }

    /**
     * Add shortest numeric alias as extension
     * 
     * @param items
     * @param domainName
     * @param user
     */
    private void addItem(Element items, String domainName, User user) {
        if (user.hasNumericUsername()) {
            // no need add those users
            return;
        }
        String extension = user.getExtension(false);
        // only generate an empty if extensio exist and extension is different from user name
        if (extension != null) {
            Element item = items.addElement("item");
            item.addElement("uri").setText(user.getUri(domainName));
            // add first found numeric alias
            item.addElement("extension").setText(extension);
        }
    }

    protected DataSet getType() {
        return DataSet.EXTENSION;
    }
}
