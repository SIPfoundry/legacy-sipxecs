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
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class CredentialsTest extends XMLTestCase {
    public void testGenerateEmpty() throws Exception {
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getDomainName();
        expectLastCall().andReturn("host.company.com");
        coreContext.getAuthorizationRealm();
        expectLastCall().andReturn("company.com");
        coreContext.loadUsers();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        User user = new User();
        user.setSipPassword("test");
        user.setUserName("user");
        for (SpecialUserType u : SpecialUserType.values()) {
            coreContext.getSpecialUser(u);
            expectLastCall().andReturn(user);            
        }

        CallGroupContext callGroupContext = createMock(CallGroupContext.class);
        callGroupContext.getCallGroups();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        replay(coreContext, callGroupContext);

        Credentials credentials = new Credentials();
        credentials.setCoreContext(coreContext);
        credentials.setCallGroupContext(callGroupContext);

        Document document = credentials.generate();

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("credential", "/items/@type", domDoc);
        // 3 special users
        assertXpathExists("/items/item[1]", domDoc);
        assertXpathEvaluatesTo("sip:user@host.company.com", "/items/item[1]/uri", domDoc);

        assertXpathExists("/items/item[2]", domDoc);
        assertXpathEvaluatesTo("sip:user@host.company.com", "/items/item[2]/uri", domDoc);

        assertXpathExists("/items/item[3]", domDoc);
        assertXpathEvaluatesTo("sip:user@host.company.com", "/items/item[3]/uri", domDoc);

        assertXpathExists("/items/item[4]", domDoc);
        assertXpathEvaluatesTo("sip:user@host.company.com", "/items/item[4]/uri", domDoc);

        assertXpathNotExists("/items/item[5]", domDoc);
        verify(coreContext, callGroupContext);
    }

    public void testAddCallgroup() throws Exception {
        Document document = DocumentFactory.getInstance().createDocument();
        Element item = document.addElement("items");

        CallGroup cg = new CallGroup();
        cg.setName("sales");
        cg.setSipPassword("pass4321");

        Credentials credentials = new Credentials();
        credentials.addCallGroup(item, cg, "sipx.sipfoundry.org", "sipfoundry.org");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("sip:sales@sipx.sipfoundry.org", "/items/item/uri", domDoc);
        String digest = "282e44b75e1e04d379d3157c34e31814";
        // Md5Encoder.digestPassword("sales", "sipfoundry.org", "pass4321");
        assertXpathEvaluatesTo(digest, "/items/item/pintoken", domDoc);
        assertXpathEvaluatesTo(digest, "/items/item/passtoken", domDoc);
        assertXpathEvaluatesTo("sipfoundry.org", "/items/item/realm", domDoc);
        assertXpathEvaluatesTo("DIGEST", "/items/item/authtype", domDoc);
    }

    public void testAddUser() throws Exception {
        Document document = DocumentFactory.getInstance().createDocument();
        Element item = document.addElement("items");
        User user = new User();
        user.setUserName("superadmin");
        final String PIN = "pin1234";
        user.setPin(PIN, "sipfoundry.org");
        user.setSipPassword("pass4321");

        Credentials credentials = new Credentials();
        credentials.addUser(item, user, "sipx.sipfoundry.org", "sipfoundry.org");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("sip:superadmin@" + "sipx.sipfoundry.org", "/items/item/uri",
                domDoc);
        assertXpathEvaluatesTo(Md5Encoder.digestPassword("superadmin", "sipfoundry.org", PIN),
                "/items/item/pintoken", domDoc);
        assertXpathEvaluatesTo(Md5Encoder.digestPassword("superadmin", "sipfoundry.org",
                "pass4321"), "/items/item/passtoken", domDoc);
        assertXpathEvaluatesTo("sipfoundry.org", "/items/item/realm", domDoc);
        assertXpathEvaluatesTo("DIGEST", "/items/item/authtype", domDoc);
    }

    public void testAddUserEmptyPasswords() throws Exception {
        Document document = DocumentFactory.getInstance().createDocument();
        Element item = document.addElement("items");

        User user = new User();
        user.setUserName("superadmin");
        user.setPin("", "sipfoundry.org");

        Credentials credentials = new Credentials();
        credentials.addUser(item, user, "sipx.sipfoundry.org", "sipfoundry.org");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("sip:superadmin@" + "sipx.sipfoundry.org", "/items/item/uri",
                domDoc);
        String emptyHash = Md5Encoder.digestPassword("superadmin", "sipfoundry.org", "");
        assertXpathEvaluatesTo(emptyHash, "/items/item/pintoken", domDoc);
        assertXpathEvaluatesTo(emptyHash, "/items/item/passtoken", domDoc);
        assertXpathEvaluatesTo("sipfoundry.org", "/items/item/realm", domDoc);
        assertXpathEvaluatesTo("DIGEST", "/items/item/authtype", domDoc);
    }
}
