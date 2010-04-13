/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

public class SimpleDefinitionsSettingEvaluatorTest extends TestCase {
    private SimpleDefinitionsEvaluator m_evaluator;

    public void testIsExpresionTrue() {
        Set defines = new HashSet<String>();
        defines.add("feature_1");
        defines.add("feature_2");
        defines.add("feature_3");

        m_evaluator = new SimpleDefinitionsEvaluator(defines);

        // checks simple expression
        String expression1 = "feature_1";
        assertTrue(m_evaluator.isExpressionTrue(expression1, null));
        String expression2 = "feature_x";
        assertFalse(m_evaluator.isExpressionTrue(expression2, null));

        // checks expression with || operator
        String expression3 = "feature_x||feature_y";
        assertFalse(m_evaluator.isExpressionTrue(expression3, null));
        String expression4 = "feature_x||feature_2||feature_y";
        assertTrue(m_evaluator.isExpressionTrue(expression4, null));
        String expression5 = "feature_x|feature_2||feature_y";
        assertFalse(m_evaluator.isExpressionTrue(expression5, null));
        String expression6 = "feature_x || feature_2 || feature_y";
        assertTrue(m_evaluator.isExpressionTrue(expression6, null));
    }
}
