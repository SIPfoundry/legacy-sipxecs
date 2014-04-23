/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.authcode;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.acccode.AuthCode;
import org.sipfoundry.sipxconfig.acccode.AuthCodeManager;
import org.sipfoundry.sipxconfig.acccode.AuthCodeSettings;
import org.sipfoundry.sipxconfig.acccode.AuthCodes;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.SameExtensionException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

/*
 * AuthCodesSettings tests are also here
 */
public class AuthCodeManagerImplTestIntegration extends IntegrationTestCase {

    private AuthCodeManager m_authCodeManager;
    private AuthCodes m_authCodes;
    private CoreContext m_coreContext;

    public void testDeleteAuthCode() throws Exception {
        loadDataSet("authcode/auth_code.db.xml");

        assertEquals(2, m_authCodeManager.getAuthCodes().size());

        AuthCode code = m_authCodeManager.getAuthCode(1);
        assertNotNull(code);
        m_authCodeManager.deleteAuthCode(code);

        assertEquals(1, m_authCodeManager.getAuthCodes().size());
    }

    public void testDeleteAuthCodes() throws Exception {
        loadDataSet("authcode/auth_code.db.xml");

        assertEquals(2, m_authCodeManager.getAuthCodes().size());
        assertNotNull(m_authCodeManager.getAuthCodeByCode("12345"));

        Collection<Integer> ids = new ArrayList<Integer>();
        ids.add(1);
        ids.add(2);

        m_authCodeManager.deleteAuthCodes(ids);

        assertEquals(0, m_authCodeManager.getAuthCodes().size());
    }

    public void testNewAuthCode() throws Exception {
        AuthCode authCode = m_authCodeManager.newAuthCode();
        assertNotNull(authCode.getInternalUser().getSipPassword());
        assertEquals(false, authCode.getInternalUser().getSettingTypedValue(PermissionName.VOICEMAIL.getPath()));
        assertEquals(false,
                authCode.getInternalUser().getSettingTypedValue(PermissionName.FREESWITH_VOICEMAIL.getPath()));
    }

    public void testCheckAuthCode() throws Exception {
        loadDataSet("authcode/auth_code.db.xml");
        assertEquals(2, m_authCodeManager.getAuthCodes().size());

        // check alias
        boolean exCaught = false;
        User user = m_coreContext.newUser();
        user.setAliasesString("*82");
        user.setUserName("user1");
        user.setPin("123");
        user.setPintoken("123");
        try {
            m_coreContext.saveUser(user);
        } catch (NameInUseException e) {
            exCaught = true;
        }
        assertTrue(exCaught);
    }

    public void testAuthCodesSettings() {
        AuthCodeSettings settings = m_authCodes.getSettings();
        settings.setAuthCodeAliases("*82");
        settings.setAuthCodePrefix("*82");
        try {
            m_authCodes.saveSettings(settings);
            fail();
        } catch (SameExtensionException e) {
        }
        settings.setAuthCodePrefix("*821");
        m_authCodes.saveSettings(settings);
    }

    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setAuthCodes(AuthCodes authCodes) {
        m_authCodes = authCodes;
    }

}
