/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.beans.factory.annotation.Required;

public class ValidUsersConfig extends XmlFile {

    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/validusers-00-00";

    private static final String ELEMENT_NAME_ALIAS = "alias";
    private static final String ELEMENT_NAME_ALIASES = "aliases";
    private static final String ELEMENT_NAME_CONTACT = "contact";
    private static final String ELEMENT_NAME_DISPLAYNAME = "displayName";
    private static final String ELEMENT_NAME_IDENTITY = "identity";
    private static final String ELEMENT_NAME_INDIRECTORY = "inDirectory";
    private static final String ELEMENT_NAME_PINTOKEN = "pintoken";
    private static final String ELEMENT_NAME_USER = "user";
    private static final String ELEMENT_NAME_USERNAME = "userName";
    
    private CoreContext m_coreContext;

    private DomainManager m_domainManager;

    private AliasProvider m_aliasProvider;


    
    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        QName validUsersName = FACTORY.createQName("validusers", NAMESPACE);
        Element usersEl = document.addElement(validUsersName);
        // FIXME: should be paging here...
        List<User> users = m_coreContext.loadUsers();

        // generate the users
        for (User user : users) {
            generateUser(usersEl, user);
        }

        // Load up the specified aliases
        List<AliasMapping> aliasMappings = (List<AliasMapping>) m_aliasProvider
                .getAliasMappings();
        // Generate the aliases
        for (AliasMapping am : aliasMappings) {
            generateAlias(usersEl, am);
        }
        return document;
    }

    private void generateUser(Element usersEl, User user) {
        String domainName = m_domainManager.getDomain().getName();

        Element userEl = usersEl.addElement(ELEMENT_NAME_USER);
        String identity = AliasMapping.createUri(user.getUserName(), domainName);
        String contact = user.getUri(domainName);
        userEl.addElement(ELEMENT_NAME_IDENTITY).setText(identity);
        userEl.addElement(ELEMENT_NAME_USERNAME).setText(user.getUserName());
        Element aliasesEl = userEl.addElement(ELEMENT_NAME_ALIASES);
        for (String alias : user.getAliases()) {
            aliasesEl.addElement(ELEMENT_NAME_ALIAS).setText(alias);
        }
        String displayName = user.getDisplayName();
        if (displayName != null) {
            userEl.addElement(ELEMENT_NAME_DISPLAYNAME).setText(displayName);
        }
        userEl.addElement(ELEMENT_NAME_CONTACT).setText(contact);
        userEl.addElement(ELEMENT_NAME_PINTOKEN).setText(user.getPintoken());
        boolean inDirectory = user.hasPermission(PermissionName.AUTO_ATTENDANT_DIALING);
        userEl.addElement(ELEMENT_NAME_INDIRECTORY).setText(Boolean.toString(inDirectory));
    }

    private void generateAlias(Element usersEl, AliasMapping am) {
        Element userEl = usersEl.addElement(ELEMENT_NAME_USER);
        String identity = am.getIdentity();
        String contact = am.getContact();
        userEl.addElement(ELEMENT_NAME_IDENTITY).setText(identity);
        userEl.addElement(ELEMENT_NAME_USERNAME).setText(identity.substring(0, identity.indexOf('@')));
        userEl.addElement(ELEMENT_NAME_CONTACT).setText(contact);
        userEl.addElement(ELEMENT_NAME_INDIRECTORY).setText("false");
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setAliasProvider(AliasProvider aliasProvider) {
        m_aliasProvider = aliasProvider;
    }
}
