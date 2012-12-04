/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import org.apache.commons.lang.StringUtils;

/**
 * This filter will escape the 5 XML entities; It will leave other characters as they are, since
 * Polycom xmls are UTF encoded. (see XX-10518)
 */
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
        String escapedXml = StringUtils.replace(value, "&", "&amp;");
        escapedXml = StringUtils.replace(escapedXml, "<", "&lt;");
        escapedXml = StringUtils.replace(escapedXml, ">", "&gt;");
        escapedXml = StringUtils.replace(escapedXml, "'", "&apos;");
        escapedXml = StringUtils.replace(escapedXml, "\"", "&quot;");
        if (value.equals(escapedXml)) {
            return sv;
        }
        return new SettingValueImpl(escapedXml);
    }
}
