/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class ConditionalSetTest extends TestCase {
    private ConditionalSet m_root;
    private Set m_definitions;
    private SettingExpressionEvaluator m_evaluator;

    protected void setUp() throws Exception {
        XmlModelBuilder builder = new XmlModelBuilder(TestHelper.getSettingModelContextRoot());
        File in = TestHelper.getResourceAsFile(getClass(), "conditionals.xml");
        m_root = (ConditionalSet) builder.buildModel(in);
        m_definitions = new HashSet();
        m_evaluator = new SimpleDefinitionsEvaluator(m_definitions);
    }

    public void testIf() throws Exception {
        m_definitions.add("hairy");
        assertNotNull(m_root.getSetting("man/shave"));
        Setting actual = m_root.evaluate(m_evaluator);
        assertNotNull(actual.getSetting("man/shave"));

        m_definitions.add("neandrathal");
        Setting actual2 = m_root.evaluate(m_evaluator);
        assertNull(actual2.getSetting("man/shave"));

        m_definitions.add("vegitarian");
        Setting actual3 = m_root.evaluate(m_evaluator);
        assertNull(actual3.getSetting("human/eat/hamburger"));
        assertNotNull(actual3.getSetting("human/eat/vegiburger"));
    }

    public void testGroupIf() throws Exception {
        Setting actual = m_root.evaluate(m_evaluator);
        assertNull(actual.getSetting("alien"));
        assertNotNull(actual.getSetting("et"));

        m_definitions.add("borg");
        Setting actual2 = m_root.evaluate(m_evaluator);
        assertNotNull(actual2.getSetting("alien"));
    }
}
