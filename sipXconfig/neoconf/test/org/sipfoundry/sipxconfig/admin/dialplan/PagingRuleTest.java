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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

public class PagingRuleTest extends TestCase {
    private PagingRule m_rule;

    protected void setUp() {
        m_rule = new PagingRule("*77", "alert");
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(1, patterns.length);
        assertEquals("*77" + ".", patterns[0]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("{vdigits}", transform.getUser());
        assertEquals("Alert-info=alert", transform.getUrlParams()[0]);
        assertEquals("${PAGE_SERVER_ADDR}:${PAGE_SERVER_SIP_PORT}", transform.getHost());
        assertNull(transform.getHeaderParams());
    }
}
