/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

/**
 * ChangePinTestUi
 */
public class ChangePinTestUi extends WebTestCase {
    private static final String CURRENT_PIN = TestPage.TEST_USER_PIN;
    private static final String NEW_PIN = "5678";
    private static final String WRONG_PIN = "0000";
    private static final String NON_NUMERIC_PIN = "nerf";

    private static final String MSG_WRONG_PIN = "The current PIN that you entered is incorrect";
    private static final String MSG_PIN_MISMATCH = "The new PIN and confirmed new PIN don't match";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ChangePinTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCoreContext");
        SiteTestHelper.seedUser(getTester());
        tester.clickLink("loginFirstTestUser");
        SiteTestHelper.home(getTester());
        clickLink("ChangePin");
    }

    public void testDisplayChangePin() throws Exception {
        assertTextNotPresent("An exception has occurred.");
        assertTextPresent("Change PIN");
    }

    public void testChangePin() throws Exception {
        // Change the PIN to a new value
        changePin(CURRENT_PIN, NEW_PIN);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        // Change the PIN back to its old value
        changePin(NEW_PIN, CURRENT_PIN);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }

    // When entering an empty current PIN, we expect to see an error that it is the wrong PIN,
    // because the PIN is not empty.  We should not get an error about the PIN being required.
    // We allow an empty current PIN even though the UI won't let you set the PIN empty,
    // because the PIN might be empty due to a database reset.
    public void testEmptyCurrentPin() throws Exception {
        changePin("", NEW_PIN);
        assertTextPresent(MSG_WRONG_PIN);
        SiteTestHelper.assertNoException(tester);
    }

    public void testEmptyNewPin() throws Exception {
        changePin(CURRENT_PIN, "");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertUserError(tester);
    }

    public void testWrongPin() throws Exception {
        changePin(WRONG_PIN, NEW_PIN);
        assertTextPresent(MSG_WRONG_PIN);
        SiteTestHelper.assertNoException(tester);
    }

    // Non-numeric PINs should work fine
    public void testNonNumericPin() throws Exception {
        changePin(CURRENT_PIN, NON_NUMERIC_PIN);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }

    public void testNewPinMismatch() throws Exception {
        changePin(CURRENT_PIN, NEW_PIN, WRONG_PIN);
        assertTextPresent(MSG_PIN_MISMATCH);
        SiteTestHelper.assertNoException(tester);
    }

    private void changePin(String oldPin, String newPin, String newPinRepeated) {
        setTextField("changePin:currentPin", oldPin);
        setTextField("cp:password", newPin);
        setTextField("cp:confirmPassword", newPinRepeated);
        clickButton("form:apply");
    }

    private void changePin(String oldPin, String newPin) {
        changePin(oldPin, newPin, newPin);
    }
}
