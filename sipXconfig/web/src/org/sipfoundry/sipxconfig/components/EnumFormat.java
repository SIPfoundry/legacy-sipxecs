/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.apache.hivemind.Messages;

public class EnumFormat extends Format {
    private Messages m_messages;

    private String m_prefixSeparator = ".";

    private String m_prefix = StringUtils.EMPTY;

    public void setMessages(Messages messages) {
        m_messages = messages;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public void setPrefixSeparator(String prefixSeparator) {
        m_prefixSeparator = prefixSeparator;
    }

    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos_) {
        Enum value = (Enum) obj;
        String name = value.getName();
        if (m_messages == null) {
            toAppendTo.append(name);
        } else {
            String nameKey = name.replaceAll(" ", "_");
            String key = m_prefix + m_prefixSeparator + nameKey;
            toAppendTo.append(m_messages.getMessage(key));
        }
        return toAppendTo;
    }

    public Object parseObject(String source, ParsePosition pos) {
        throw new UnsupportedOperationException();
    }
}
