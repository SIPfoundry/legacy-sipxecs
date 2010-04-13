/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import junit.framework.TestCase;

public class DomainDialingRuleTest extends TestCase {
    private Domain m_domain;
    private DomainDialingRule m_rule;

    protected void setUp() {
        m_domain = new Domain("example.org");
        m_domain.addAlias("alias.example.org");
        m_rule = new DomainDialingRule(m_domain);
    }

    public void testGetTransforms() {
        assertEquals(1, m_rule.getTransforms().length);
    }

    public void testGetHostPatterns() {
        String[] hostPatterns = m_rule.getHostPatterns();
        assertEquals(1, hostPatterns.length);
        assertSame("alias.example.org", hostPatterns[0]);
    }

}
