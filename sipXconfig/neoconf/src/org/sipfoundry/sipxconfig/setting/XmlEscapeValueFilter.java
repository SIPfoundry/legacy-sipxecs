/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import org.apache.commons.lang.StringEscapeUtils;

public class XmlEscapeValueFilter implements SettingValueFilter {
    public static final XmlEscapeValueFilter FILTER = new XmlEscapeValueFilter();

    public SettingValue filter(SettingValue sv) {
        if (sv == null) {
            return null;
        }
        String value = sv.getValue();
        if (value == null) {
            return sv;
        }
        String escapedXml = StringEscapeUtils.escapeXml(value);
        if (value.equals(escapedXml)) {
            return sv;
        }
        return new SettingValueImpl(escapedXml);
    }
}
