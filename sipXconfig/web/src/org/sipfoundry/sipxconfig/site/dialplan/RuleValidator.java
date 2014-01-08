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

import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.BaseValidator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.commons.util.HolidayPeriod;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;

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
        if (rule instanceof AttendantRule) {
            AttendantRule attendantRule = (AttendantRule) rule;
            List<HolidayPeriod> periods = attendantRule.getHolidayAttendant()
                    .getPeriods();
            for (HolidayPeriod period : periods) {
                if (period.getEndDate().before(period.getStartDate())) {
                    IPage page = field.getPage();
                    throw new ValidatorException(page.getMessages().getMessage(
                            "validationMessage.inconsistentHolidayDates"),
                            ValidationConstraint.CONSISTENCY);
                }
            }
        }
    }
}
