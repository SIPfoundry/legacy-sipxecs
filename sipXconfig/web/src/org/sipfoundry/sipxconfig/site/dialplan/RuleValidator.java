/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.BaseValidator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class RuleValidator extends BaseValidator {

    public void validate(IFormComponent field, ValidationMessages messages, Object object)
        throws ValidatorException {
        IDialingRule rule = (IDialingRule) object;
        if (rule == null) {
            // no rule to validate at this time
            return;
        }
        if (!rule.isEnabled()) {
            // only validate enabled rules
            return;
        }
        if (!rule.isEnablable()) {
            // rule is invalid - external rules have to have gateways
            rule.setEnabled(false);
            throw new ValidatorException(getMessage(), ValidationConstraint.CONSISTENCY);
        }
    }
}
