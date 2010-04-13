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

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.site.UserSession;

/**
 * ChangePin
 */

public abstract class ChangePin extends SipxBasePage {

    /**
     * Properties
     */

    public abstract CoreContext getCoreContext();
    public abstract LoginContext getLoginContext();

    public abstract String getCurrentPin();
    public abstract void setCurrentPin(String currentPin);

    public abstract String getNewPin();
    public abstract void setNewPin(String newPin);
    public abstract UserSession getUserSession();

    /**
     * Listeners
     */

    public void changePin() {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        // Get the userId.  Note that the Border component of the page ensures
        // that the user is logged in and therefore that userId is non-null.
        Integer userId = getUserSession().getUserId();

        // Validate the current PIN.
        // Note that the ConfirmPassword component ensures that the new PIN and
        // confirm new PIN fields match, so we don't have to worry about that here.

        CoreContext coreContext = getCoreContext();
        User user = coreContext.loadUser(userId);
        LoginContext loginContext = getLoginContext();

        // If the currentPin is null, then make it the empty string
        String currentPin = (String) ObjectUtils.defaultIfNull(getCurrentPin(), StringUtils.EMPTY);

        user = loginContext.checkCredentials(user.getUserName(), currentPin);
        if (user == null) {
            IValidationDelegate delegate = TapestryUtils.getValidator(this);
            delegate.record(getMessages().getMessage("error.badCurrentPin"), ValidationConstraint.CONSISTENCY);
            return;
        }

        // Change the PIN
        user.setPin(getNewPin(), coreContext.getAuthorizationRealm());
        coreContext.saveUser(user);

        // Scrub the PIN fields, for security
        setCurrentPin(null);
        setNewPin(null);
    }

}
