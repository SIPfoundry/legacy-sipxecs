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

import java.util.Set;

/**
 * look for expression in the set
 */
public class SimpleDefinitionsEvaluator implements SettingExpressionEvaluator {
    private Set m_defines;

    public SimpleDefinitionsEvaluator(Set defines) {
        m_defines = defines;
    }

    public boolean isExpressionTrue(String expression, Setting setting_) {
        return m_defines.contains(expression);
    }
}
