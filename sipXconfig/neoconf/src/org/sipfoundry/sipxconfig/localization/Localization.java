/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.localization;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class Localization extends BeanWithId {

    private String m_region;

    private String m_language;

    public String getRegion() {
        return m_region;
    }

    public void setRegion(String region) {
        m_region = region;
    }

    public String getLanguage() {
        return m_language;
    }

    public void setLanguage(String language) {
        m_language = language;
    }

    public String getRegionId() {
        return getIdFromString(m_region);
    }

    public static String getIdFromString(String string) {
        String[] split = StringUtils.split(string, "_");
        if (split == null || split.length == 0) {
            return null;
        }
        return split[split.length - 1];
    }
}
