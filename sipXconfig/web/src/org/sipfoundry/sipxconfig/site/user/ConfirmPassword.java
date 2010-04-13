/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ConfirmPassword extends BaseComponent {

    public abstract String getPassword();
    public abstract void setPassword(String password);

    public abstract String getConfirmPassword();
    public abstract void setConfirmPassword(String confirmPassword);

    public abstract String getPasswordMismatchMessage();
    public abstract void setPasswordMismatchMessage(String passwordMismatchMessage);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (!cycle.isRewinding()) {
            // If the password is null, then init both password and confirmPassword to the empty string
            if (getPassword() == null) {
                setPassword(StringUtils.EMPTY);
                setConfirmPassword(StringUtils.EMPTY);
            }

            // If the confirmPassword is null, then init it to be the same as the password
            setConfirmPassword((String) ObjectUtils.defaultIfNull(getConfirmPassword(), getPassword()));
        }

        super.renderComponent(writer, cycle);

        // At rewind time, after the user has filled in the form, do any validation that spans
        // multiple fields.  Such validation cannot simply use a validator bound to a single field.
        if (cycle.isRewinding()) {
            // The user typed in the password twice.  Make sure that it was the same both times.
            IValidationDelegate delegate =
                TapestryUtils.getValidator(cycle.getPage());
            String password = StringUtils.defaultIfEmpty(getPassword(), StringUtils.EMPTY);
            String confirmPassword = StringUtils.defaultIfEmpty(getConfirmPassword(), StringUtils.EMPTY);
            if (!password.equals(confirmPassword)) {
                delegate.record(getPasswordMismatchMessage(), ValidationConstraint.CONSISTENCY);
                return;
            }
        }
    }

}
