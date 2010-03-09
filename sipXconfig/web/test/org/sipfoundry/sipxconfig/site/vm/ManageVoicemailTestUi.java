/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;


import java.text.DateFormat;
import java.util.Locale;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;
import org.sipfoundry.sipxconfig.test.PhonebookTestHelper;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;


public class ManageVoicemailTestUi extends WebTestCase {
    private static final int PORTAL_USER_ID = 1;
    private User m_portalUser;
    private ManageVoicemail m_out;
    private Creator m_pageCreator;
    private CoreContext m_coreContext;
    private DomainManager m_domainManager;
    private PhonebookTestHelper m_testHelper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageVoicemailTestUi.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        m_portalUser = new User();
        m_portalUser.setName("portalUser");
        m_testHelper = new PhonebookTestHelper();

        m_pageCreator = new Creator();
        m_out = (ManageVoicemail) m_pageCreator.newInstance(ManageVoicemail.class);

        m_out.setUserId(PORTAL_USER_ID);
        m_coreContext = createMock(CoreContext.class);
        m_testHelper.configureCoreContextMock(m_coreContext);

        m_coreContext.loadUser(PORTAL_USER_ID);
        expectLastCall().andReturn(m_portalUser).anyTimes();



        Domain domain = new Domain();
        domain.setName("example.com");
        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomain();
        expectLastCall().andReturn(domain).anyTimes();

        PropertyUtils.write(m_out, "coreContext", m_coreContext);
        PropertyUtils.write(m_out, "domainManager", m_domainManager);

        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        replay(m_coreContext, m_domainManager);
    }

    private void gotoManageVoicemail() {
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("seedLocationsManager");
        clickLink("resetVoicemail");
        clickLink("loginFirstTestUser");
        clickLink("ManageVoicemail");
    }

    public void testNoPermissionNotice() {
        SiteTestHelper.home(getTester());
        clickLink("DisableVoicemailPermission");
        clickLink("ManageVoicemail");
        assertElementPresent("noVoicemailPermission");
    }

    public void testDisplay() throws Exception {
        gotoManageVoicemail();
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertElementPresent("voicemail:list");
    }

    public void testNavigation() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
    }

    public void XX7777_tmp_disable_testMove() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("Voice Message 00000002");
        checkCheckbox("checkbox");
        actionSelection(MoveVoicemailAction.class, "deleted");
        assertTextNotPresent("Voice Message 00000002");
        clickLink("link:deleted");
        assertTextPresent("Voice Message 00000001");
        assertTextPresent("Voice Message 00000002");
    }

    private void actionSelection(Class groupAction, String action) {
        selectOptionByValue("actionSelection", groupAction.getName() + action);
    }

    public void testEdit() throws Exception {
        gotoManageVoicemail();
        clickLinkWithText("Voice Message 00000002");
        assertElementPresent("voicemail:edit");
    }

    public void testPageServiceShouldntRedirect() throws Exception {
        SiteTestHelper.home(getTester());
        clickLink("seedTestUser");
        clickLink("Logout");
        clickLink("ManageVoicemail");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementNotPresent("voicemail:edit");
    }

    public void testExternalServiceShouldRedirect() throws Exception {
        SiteTestHelper.home(getTester());
        clickLink("seedTestUser");
        clickLink("Logout");
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }

    public void testExternalServiceShouldRedirectSecondPasswordAttempt() throws Exception {
        SiteTestHelper.home(getTester());
        clickLink("seedTestUser");
        clickLink("Logout");
        gotoPage(String.format("mailbox/%s/inbox", TestPage.TEST_USER_USERNAME));
        login(TestPage.TEST_USER_USERNAME, "Bogus");
        login(TestPage.TEST_USER_USERNAME, TestPage.TEST_USER_PIN);
        assertElementPresent("voicemail:list");
    }

    public void _testForwardedMessages() throws Exception {
        // FIXME text returned from table contains \n
        DateFormat fmt = TapestryUtils.getDateFormat(Locale.getDefault());
        gotoManageVoicemail();
        String[][] forwardedMessage = {{
            "unchecked",
            "Fwd:Voice Message 00000014 \n Voice Message 00000014",
            "200 \n Yellowthroat Warbler - 200",
            fmt.format(TestUtil.localizeDateTime("2/9/07 6:03:00 PM EST")) + " "
              + fmt.format(TestUtil.localizeDateTime("2/9/07 3:40:00 PM EST")),
            "6 seconds",
            ""
        }};

        assertTableRowsEqual("voicemail:list", 3, forwardedMessage);
        checkCheckbox("checkbox_0");
        actionSelection(MoveVoicemailAction.class, "saved");
        clickLink("link:saved");

        assertTableRowsEqual("voicemail:list", 2, forwardedMessage);
    }

    public void testPlayFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        gotoPage(String.format("mailbox/%s/inbox/00000002", TestPage.TEST_USER_USERNAME));
// FIXME: how to check content type?
//        assertMatchInElement(elementID, regexp) Equals("audio/x-wav", getServeurResponse().getContentType());
    }

    public void XX7777_tmp_disable_testDeleteFriendlyUrl() throws Exception {
        gotoManageVoicemail();
        assertTextPresent("00000002");
        gotoPage(String.format("mailbox/%s/inbox/00000002/delete", TestPage.TEST_USER_USERNAME));
        assertTextNotPresent("00000002");
    }

    public void testCall() {
        SipService sipService = createMock(SipService.class);
        String fromUri = "'200'<sip:123@example.com>";
        sipService.sendRefer(m_portalUser, "sip:portalUser@example.com", "ClickToCall", "sip:123@example.com");
        replay(sipService);

        PropertyUtils.write(m_out, "sipService", sipService);

        m_out.call(fromUri);

        verify(sipService);
    }
    /**
     *Verify that both <audio and <img tag are present in ManageVoicemail.html
     *The browser will display either <audio tag (if supported) or <a and <img tag
     *(if HTML5 audio tag is not supported)
     */
    public void testPlay() {
        gotoManageVoicemail();
        assertElementPresentByXPath("//input[@type = 'image' and @id='ImageSubmit']");
        clickElementByXPath("//input[@type = 'image' and @id='ImageSubmit']");
        assertElementPresentByXPath("//audio[@autoplay = 'true']");
        assertElementPresentByXPath("//img[@src = '/sipxconfig/images/play.png']");
    }

    private void login(String username, String password) {
        assertElementPresent("loginForm");
        setTextField("j_username", username);
        setTextField("j_password", password);
        clickButton("login:submit");
    }
}


