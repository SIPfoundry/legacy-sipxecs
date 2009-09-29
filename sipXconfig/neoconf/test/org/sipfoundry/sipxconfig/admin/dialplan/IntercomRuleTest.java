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

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

import junit.framework.TestCase;

public class IntercomRuleTest extends TestCase {
    private IntercomRule m_rule;

    protected void setUp() {
        m_rule = new IntercomRule(true, "*78", "Ipek", 42);
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(1, patterns.length);
        assertEquals("*78" + ".", patterns[0]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("{vdigits}", transform.getUser());
        String[] headerparams = transform.getHeaderParams();
        assertEquals(2, headerparams.length);
        assertEquals("Alert-info=Ipek", headerparams[0]);
        assertEquals("Call-Info=<sip:localhost>;answer-after=42", headerparams[1]);
        assertNull(transform.getHost());
        assertNull(transform.getUrlParams());
    }
}
