/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.Arrays;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ValidUsersConfigTest extends XMLTestCase {

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerate() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User u1 = new User();
        u1.setPermissionManager(pm);
        u1.setName("301");
        u1.setFirstName("John");
        u1.setLastName("Adams");
        u1.setPin("1234", "example");
        u1.setPermission(PermissionName.AUTO_ATTENDANT_DIALING, true);
        u1.setAliasesString("jdoe 18003010000");

        User u2 = new User();
        u2.setPermissionManager(pm);
        u2.setName("302");
        u2.setFirstName("George");
        u2.setLastName("Washington");
        u2.setPin("1234", "example");
        u2.setPermission(PermissionName.AUTO_ATTENDANT_DIALING, false);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsers();
        expectLastCall().andReturn(Arrays.asList(u1, u2));

        Domain domain = new Domain();
        domain.setName("example.com");

        DomainManager domainManager = createMock(DomainManager.class);
        expect(domainManager.getDomain()).andReturn(domain).anyTimes();

        replay(coreContext, domainManager);

        ValidUsersConfig vu = new ValidUsersConfig();
        vu.setCoreContext(coreContext);
        vu.setDomainManager(domainManager);

        String generatedXml = vu.getFileContent();
        InputStream referenceXml = getClass().getResourceAsStream("validusers.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        verify(coreContext, domainManager);
    }
}
