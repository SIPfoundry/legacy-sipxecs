/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Set;

import static org.apache.commons.lang.StringUtils.splitByWholeSeparator;
import static org.apache.commons.lang.StringUtils.trim;

/**
 * Look for expression in the set: it supports alternative (denoted as "||" operator)
 */
public class SimpleDefinitionsEvaluator implements SettingExpressionEvaluator {
    private final Set m_defines;

    public SimpleDefinitionsEvaluator(Set defines) {
        m_defines = defines;
    }

    public boolean isExpressionTrue(String expression, Setting setting_) {
        for (String token : splitByWholeSeparator(expression, "||")) {
            if (m_defines.contains(trim(token))) {
                return true;
            }
        }
        return false;
    }
}
