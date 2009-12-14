/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class PresenceRoutingConfiguration extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/presencerouting-config-00-00";

    private CoreContext m_coreContext;

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element userPrefs = document.addElement("presenceRoutingPrefs", NAMESPACE);

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generateUserPrefXml(user, userPrefs);
            }
        };
        forAllUsersDo(m_coreContext, closure);

        return document;
    }

    private void generateUserPrefXml(User user, Element userPrefs) {
        ImAccount imAccount = new ImAccount(user);
        Element userElement = userPrefs.addElement("user");
        userElement.addElement("userName").setText(user.getUserName());
        userElement.addElement("vmOnDnd").setText(Boolean.toString(imAccount.isForwardOnDnd()));
    }
}
