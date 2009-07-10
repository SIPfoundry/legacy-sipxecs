/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

import java.util.ArrayList;
import java.util.List;

import static org.apache.commons.lang.StringUtils.join;
import static org.apache.commons.lang.StringUtils.split;
import static org.apache.commons.lang.StringUtils.trimToNull;

public class MultiEnumSetting extends EnumSetting {

    /**
     * Used to format internal representation.
     */
    private String m_separator = "|";

    public MultiEnumSetting() {
    }

    @Override
    public String getName() {
        return "multiEnum";
    }

    @Override
    public Object convertToTypedValue(Object value) {
        if (value instanceof String) {
            String[] strings = split((String) value, m_separator);
            ArrayList values = new ArrayList(strings.length);
            for (String s : strings) {
                Object v = super.convertToTypedValue(s);
                if (v != null) {
                    values.add(v);
                }
            }
            return values;
        }
        return null;
    }

    @Override
    public String convertToStringValue(Object value) {
        if (value instanceof List) {
            List vals = (List) value;
            List<String> strings = new ArrayList<String>();
            for (Object v : vals) {
                String s = super.convertToStringValue(v);
                if (s != null) {
                    strings.add(s);
                }
            }

            return trimToNull(join(strings, m_separator));
        }
        return super.convertToStringValue(value);
    }

    public void setSeparator(String separator) {
        m_separator = separator;
    }
}
