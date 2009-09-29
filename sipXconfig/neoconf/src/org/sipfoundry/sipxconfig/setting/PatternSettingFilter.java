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

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;


public class PatternSettingFilter implements SettingFilter {

    private List m_exclude = new ArrayList();

    public void addExcludes(String excludePath) {
        Pattern pattern = Pattern.compile(excludePath);
        m_exclude.add(pattern);
    }

    public boolean acceptSetting(Setting root_, Setting setting) {
        boolean accept = true;

        for (int i = 0; accept && i < m_exclude.size(); i++) {
            Pattern p = (Pattern) m_exclude.get(i);
            String path = setting.getPath();
            boolean match = p.matcher(path).matches();
            accept = !match;
        }

        return accept;
    }
}




