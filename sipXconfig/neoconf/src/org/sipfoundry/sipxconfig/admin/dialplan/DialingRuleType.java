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
            InternationalRule.class, CallTag.INTN);
    public static final DialingRuleType EMERGENCY = new DialingRuleType("Emergency", EmergencyRule.class,
            CallTag.EMERG);
    public static final DialingRuleType MAPPING_RULE = new DialingRuleType("Mapping Rule", MappingRule.class);
    public static final DialingRuleType CUSTOM = new DialingRuleType("Custom", CustomDialingRule.class);
    public static final DialingRuleType LOCAL = new DialingRuleType("Local", LocalRule.class, CallTag.LOCL);
    public static final DialingRuleType INTERNAL = new DialingRuleType("Internal", InternalRule.class);
    public static final DialingRuleType LONG_DISTANCE = new DialingRuleType("Long Distance", LongDistanceRule.class);
    public static final DialingRuleType RESTRICTED = new DialingRuleType("Restricted", LongDistanceRule.class,
            CallTag.REST);
    public static final DialingRuleType TOLL_FREE = new DialingRuleType("Toll free", LongDistanceRule.class,
            CallTag.TF);
    public static final DialingRuleType ATTENDANT = new DialingRuleType("Attendant", AttendantRule.class, CallTag.AA);
    public static final DialingRuleType INTERCOM = new DialingRuleType("Intercom", IntercomRule.class);
    public static final DialingRuleType PAGING = new DialingRuleType("Paging", PagingRule.class, CallTag.PAGE);
    public static final DialingRuleType SITE_TO_SITE = new DialingRuleType("Site To Site",
            SiteToSiteDialingRule.class, CallTag.STS);

    private final Class< ? extends DialingRule> m_klass;
    private final CallTag m_callTag;

    private DialingRuleType(String name, Class< ? extends DialingRule> klass) {
        this(name, klass, null);
    }

    private DialingRuleType(String name, Class< ? extends DialingRule> klass, CallTag callTag) {
        super(name);
        m_klass = klass;
        m_callTag = callTag;
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

    public CallTag getCallTag() {
        return m_callTag;
    }
}
