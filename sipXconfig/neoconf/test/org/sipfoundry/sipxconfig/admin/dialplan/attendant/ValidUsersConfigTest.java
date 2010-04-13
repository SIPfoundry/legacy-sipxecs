/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.InputStream;
import java.util.Arrays;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.AttachType;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.MailFormat;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_ATTACH_AUDIO;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_FORMAT;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_NOTIFICATION;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.IMAP_HOST;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.*;

public class ValidUsersConfigTest extends TestCase {

    public void testGenerate() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User u1 = new User();
        u1.setPermissionManager(pm);
        u1.setName("301");
        u1.setFirstName("John");
        u1.setLastName("Adams");
        u1.setPin("1234", "example");
        u1.setSipPassword("1234");
        u1.setPermission(PermissionName.AUTO_ATTENDANT_DIALING, true);
        u1.setPermission(PermissionName.VOICEMAIL, true);
        u1.setPermission(PermissionName.RECORD_SYSTEM_PROMPTS, false);
        u1.setAliasesString("jdoe 18003010000");
        u1.setEmailAddress("301@sipx.org");
        u1.setSettingValue(IMAP_HOST, "imap.host.exampl.com");
        u1.setSettingValue(IMAP_ACCOUNT, "jdoe");
        u1.setSettingValue(IMAP_PASSWORD, "password");

        User u2 = new User();
        u2.setPermissionManager(pm);
        u2.setName("302");
        u2.setFirstName("George");
        u2.setLastName("Washington");
        u2.setPin("1234", "example");
        u2.setSipPassword("1234");
        u2.setPermission(PermissionName.AUTO_ATTENDANT_DIALING, false);
        u2.setPermission(PermissionName.VOICEMAIL, true);
        u2.setPermission(PermissionName.RECORD_SYSTEM_PROMPTS, true);
        u2.setEmailAddress("302@sipx.org");
        u2.setAlternateEmailAddress("302@sipxecs.org");
        u2.setSettingValue(PRIMARY_EMAIL_NOTIFICATION, AttachType.YES.getValue());
        u2.setSettingValue(ALT_EMAIL_NOTIFICATION, AttachType.YES.getValue());
        u2.setSettingValue(ALT_EMAIL_FORMAT, MailFormat.BRIEF.name());
        u2.setSettingTypedValue(ALT_EMAIL_ATTACH_AUDIO, true);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Arrays.asList(u1, u2));

        Domain domain = new Domain();
        domain.setName("example.com");

        DomainManager domainManager = createMock(DomainManager.class);
        expect(domainManager.getDomain()).andReturn(domain).anyTimes();
        expect(domainManager.getAuthorizationRealm()).andReturn("example").anyTimes();

        AliasMapping am1 = new AliasMapping("500@example.com", "sip:500@example.com");
        AliasMapping am2 = new AliasMapping("501@example.com", "sip:501@example.com");

        AliasProvider aliasProvider = createMock(AliasProvider.class);
        expect(aliasProvider.getAliasMappings()).andReturn(Arrays.asList(am1, am2)).anyTimes();

        replay(coreContext, domainManager, aliasProvider);

        ValidUsersConfig vu = new ValidUsersConfig();
        vu.setCoreContext(coreContext);
        vu.setDomainManager(domainManager);
        vu.setAliasProvider(aliasProvider);

        String generatedXml = getFileContent(vu, null);
        InputStream referenceXml = getClass().getResourceAsStream("validusers.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);

        verify(coreContext, domainManager, aliasProvider);
    }
}
