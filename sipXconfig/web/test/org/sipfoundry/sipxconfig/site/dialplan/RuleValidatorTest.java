/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LocalRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;

import junit.framework.TestCase;

public class RuleValidatorTest extends TestCase {

    public void testValidateDisabledRule() {
        RuleValidator out = new RuleValidator();
        IDialingRule rule = new InternalRule();
        rule.setEnabled(false);
        try {
            // disabled rules are valid
            out.validate(null, null, rule);
        } catch (ValidatorException ve) {
            fail(ve.getMessage());
        }
    }

    public void testValidateEnabledInternalRule() {
        RuleValidator out = new RuleValidator();
        IDialingRule rule = new InternalRule();
        rule.setEnabled(true);
        try {
            // enabled internal rules without gateways are valid
            out.validate(null, null, rule);
        } catch (ValidatorException ve) {
            fail(ve.getMessage());
        }
    }

    public void testValidateEnabledExternalRuleWithNoGateways() {
        RuleValidator out = new RuleValidator();
        IDialingRule rule = new LocalRule();
        rule.setEnabled(true);
        try {
            // enabled external rules without gateways are invalid
            out.validate(null, null, rule);
            fail("Expected exception for enabled external rule with no gateways.");
        } catch (ValidatorException ve) {
            // expected behavior
        }
    }

    public void testValidateEnabledExternalRuleWithGateway() {
        RuleValidator out = new RuleValidator();
        LocalRule rule = new LocalRule();
        rule.addGateway(new Gateway());
        rule.setEnabled(true);
        try {
            // enabled emergency rules with gateways are valid
            out.validate(null, null, rule);
        } catch (ValidatorException ve) {
            fail(ve.getMessage());
        }
    }
}
