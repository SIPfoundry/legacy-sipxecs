/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class VoicemailRedirectRuleTest extends TestCase {

    private static final String USER_MATCH_PATTERN = ".";
    private static final String VOICEMAIL_USER = "~~vm~{user}";
    private static final Object FIELD_PARAMS = "q=0.1";

    private VoicemailRedirectRule m_out;

    public void setUp() {
        m_out = new VoicemailRedirectRule();
    }

    public void testGetPatterns() {
        String[] patterns = m_out.getPatterns();
        assertEquals(1, patterns.length);
        assertEquals(USER_MATCH_PATTERN, patterns[0]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_out.getTransforms();
        assertEquals(1, transforms.length);
        assertTrue(transforms[0] instanceof FullTransform);
        FullTransform fullTransform = (FullTransform) transforms[0];
        assertEquals(VOICEMAIL_USER, fullTransform.getUser());
        assertEquals(1, fullTransform.getFieldParams().length);
        assertEquals(FIELD_PARAMS, fullTransform.getFieldParams()[0]);
    }

    public void testGetType() {
        assertEquals(DialingRuleType.MAPPING_RULE, m_out.getType());
    }

    public void testIsEnabled() {
        assertTrue(m_out.isEnabled());
    }

    public void testGetPermissionNames() {
        List<String> permissionNames = m_out.getPermissionNames();
        assertTrue(permissionNames.contains(PermissionName.VOICEMAIL.getName()));
    }

    public void testIsInternal() {
        assertTrue(m_out.isInternal());
    }

}
