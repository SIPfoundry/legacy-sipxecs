/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.enums.Enum;

public final class DialingRuleType extends Enum {
    public static final DialingRuleType INTERNATIONAL = new DialingRuleType("International",
            InternationalRule.class);
    public static final DialingRuleType EMERGENCY = new DialingRuleType("Emergency",
            EmergencyRule.class);
    public static final DialingRuleType MAPPING_RULE = new DialingRuleType("Mapping Rule",
            MappingRule.class);
    public static final DialingRuleType CUSTOM = new DialingRuleType("Custom",
            CustomDialingRule.class);
    public static final DialingRuleType LOCAL = new DialingRuleType("Local", LocalRule.class);
    public static final DialingRuleType INTERNAL = new DialingRuleType("Internal",
            InternalRule.class);
    public static final DialingRuleType LONG_DISTANCE = new DialingRuleType("Long Distance",
            LongDistanceRule.class);
    public static final DialingRuleType RESTRICTED = new DialingRuleType("Restricted",
            LongDistanceRule.class);
    public static final DialingRuleType TOLL_FREE = new DialingRuleType("Toll free",
            LongDistanceRule.class);
    public static final DialingRuleType ATTENDANT = new DialingRuleType("Attendant",
            AttendantRule.class);
    public static final DialingRuleType INTERCOM = new DialingRuleType("Intercom",
            IntercomRule.class);

    private Class< ? extends DialingRule> m_klass;

    private DialingRuleType(String name, Class< ? extends DialingRule> klass) {
        super(name);
        m_klass = klass;
    }

    public DialingRule create() {
        try {
            return m_klass.newInstance();
        } catch (InstantiationException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }
}
