/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.StringWriter;

import org.apache.commons.logging.LogFactory;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public class ExportCsvTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_phoneContext;

    protected void setUp() throws Exception {

        m_phoneContext = (PhoneContext) TestHelper.getApplicationContext().getBean(
                            PhoneContext.CONTEXT_BEAN_NAME);
    }

    public void testExport() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/PhoneSummarySeed.xml");

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getAuthorizationRealm();
        coreContextCtrl.andReturn("sipfoundry.org").times(1);
        coreContext.getUsersCount();
        coreContextCtrl.andReturn(0).times(1);

        coreContextCtrl.replay();

        IMocksControl mailboxManagerControl = EasyMock.createControl();
        MailboxManager mailboxManager = mailboxManagerControl.createMock(MailboxManager.class);
        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(false).times(3);
        mailboxManagerControl.replay();

        ExportCsv exportCsvImpl = new ExportCsv();
        exportCsvImpl.setCoreContext(coreContext);
        exportCsvImpl.setPhoneContext(m_phoneContext);
        exportCsvImpl.setMailboxManager(mailboxManager);
        StringWriter writer = new StringWriter();
        exportCsvImpl.exportCsv(writer);
        LogFactory.getLog(getClass()).error(writer.toString());
        assertTrue(writer.toString().startsWith("User name,Voice-mail PIN,SIP password,First name,Last name,User alias,EMail address,User group,Phone serial number,Phone model,Phone group,Phone description\n"+
                "testuser,sipfoundry.org#1234,1234,Test,User,123,,,999123456,testPhoneModel,,unittest-sample phone1\n"+
                ",,,,,,,,999123457,testPhoneModel,,unittest-sample phone2\n"+
                "testuser,sipfoundry.org#1234,1234,Test,User,123,,,999123458,testPhoneModel,,unittest-sample phone3\n"+
                "testuser,sipfoundry.org#1234,1234,Test,User,123,,,999123458,testPhoneModel,,unittest-sample phone3\n"));

        coreContextCtrl.verify();
        mailboxManagerControl.verify();
    }
}
