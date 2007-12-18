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

import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;

public class PermissionsTest extends XMLTestCase {

    public void testGenerateEmpty() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User testUser = new User();
        testUser.setPermissionManager(pm);

        IMocksControl control = EasyMock.createControl();
        CoreContext coreContext = control.createMock(CoreContext.class);
        coreContext.getDomainName();
        control.andReturn("host.company.com");
        coreContext.loadUsers();
        control.andReturn(Collections.EMPTY_LIST);
        coreContext.newUser();
        control.andReturn(testUser).anyTimes();
        control.replay();

        Permissions permissions = new Permissions();
        permissions.setCoreContext(coreContext);

        Document document = permissions.generate();

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("permission", "/items/@type", domDoc);
        assertXpathExists("/items/item", domDoc);
        // 5 permissions per special user
        assertXpathEvaluatesTo("sip:~~id~park@host.company.com", "/items/item/identity", domDoc);
        assertXpathEvaluatesTo("sip:~~id~park@host.company.com", "/items/item[5]/identity",
                domDoc);
        assertXpathEvaluatesTo("sip:~~id~media@host.company.com", "/items/item[6]/identity",
                domDoc);
        assertXpathEvaluatesTo("sip:~~id~media@host.company.com", "/items/item[10]/identity",
                domDoc);
        assertXpathEvaluatesTo("sip:~~id~acd@host.company.com", "/items/item[11]/identity",
                domDoc);
        assertXpathEvaluatesTo("sip:~~id~acd@host.company.com", "/items/item[15]/identity",
                domDoc);
        assertXpathNotExists("/items/item[16]", domDoc);
        control.verify();
    }

    public void testAddUser() throws Exception {
        Document document = DocumentFactory.getInstance().createDocument();
        Element items = document.addElement("items");

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setPermissionManager(pm);

        Group g = new Group();
        PermissionName.INTERNATIONAL_DIALING.setEnabled(g, false);
        PermissionName.LONG_DISTANCE_DIALING.setEnabled(g, false);
        PermissionName.TOLL_FREE_DIALING.setEnabled(g, false);
        PermissionName.LOCAL_DIALING.setEnabled(g, true);
        PermissionName.SIPX_VOICEMAIL.setEnabled(g, false);
        PermissionName.EXCHANGE_VOICEMAIL.setEnabled(g, true);

        user.addGroup(g);
        user.setUserName("goober");

        Permissions permissions = new Permissions();
        permissions.addUser(items, user, "sipx.sipfoundry.org");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("sip:goober@sipx.sipfoundry.org", "/items/item/identity", domDoc);
        assertXpathEvaluatesTo("LocalDialing", "/items/item/permission", domDoc);

        assertXpathEvaluatesTo("sip:goober@sipx.sipfoundry.org", "/items/item[4]/identity",
                domDoc);
        assertXpathEvaluatesTo("ExchangeUMVoicemailServer", "/items/item[4]/permission", domDoc);

        assertXpathEvaluatesTo("sip:~~vm~goober@sipx.sipfoundry.org", "/items/item[5]/identity",
                domDoc);
        assertXpathEvaluatesTo("ExchangeUMVoicemailServer", "/items/item[5]/permission", domDoc);
    }
}
