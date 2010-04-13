/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.components;

import java.text.MessageFormat;

import org.apache.hivemind.Messages;

/**
 * Checks for a message in several message bundles. Returns the first found message.
 */
public class CompositeMessages implements Messages {

    private final Messages[] m_messages;

    public CompositeMessages(Messages... messages) {
        m_messages = messages;
    }

    @Override
    public String getMessage(String key) {
        for (Messages mm : m_messages) {
            String message = LocalizationUtils.getMessage(mm, key, null);
            if (message != null) {
                return message;
            }
        }
        return "[" + key.toUpperCase() + "]";
    }

    @Override
    public String format(String key, Object[] arguments) {
        String pattern = getMessage(key);
        return MessageFormat.format(pattern, arguments);
    }

    @Override
    public String format(String key, Object argument) {
        return format(key, asArray(argument));
    }

    @Override
    public String format(String key, Object argument1, Object argument2) {
        return format(key, asArray(argument1, argument2));
    }

    @Override
    public String format(String key, Object argument1, Object argument2, Object argument3) {
        return format(key, asArray(argument1, argument2, argument3));
    }

    private static Object[] asArray(Object... args) {
        return args;
    }
}
