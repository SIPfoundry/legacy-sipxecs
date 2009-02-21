/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.text.MessageFormat;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Caught in application layer, this informs the user they've done something wrong. Despite being
 * an unchecked exception, this is not meant to be a fatal error and application layer should
 * handle it gracefully.
 *
 * If error message have parameters throw it like this:
 *
 * throw new UserException("This value should be {0} and not {1}", "bongo", 5);
 *
 * If you prefix the message or the parameter with '&' the UI layer will try to localize them
 *
 * throw new UserException("&msg.key", "bongo", 5);
 *
 * The UI layer page will have to have msg.key defined somewhere in the localization bundle.
 *
 * If you just rethrowing some other exception:
 *
 * throw new UserException(e)
 *
 */
public class UserException extends RuntimeException {
    private String m_message;

    private Object[] m_params = ArrayUtils.EMPTY_OBJECT_ARRAY;

    public UserException() {
    }

    public UserException(Throwable cause) {
        super(cause);
    }

    /**
     * Create new exception
     *
     * @param message - message format (does not have to have any parameters)
     * @param params - parameters to be passed to MessageFormat when displaying exception errror
     */
    public UserException(String message, Object... params) {
        m_message = message;
        m_params = params;
    }

    @Override
    public String getMessage() {
        if (m_message != null) {
            return format(m_message, m_params);
        }
        if (getCause() != null) {
            return getCause().getLocalizedMessage();
        }
        return StringUtils.EMPTY;
    }

    public String format(String msgFormat, Object... params) {
        return MessageFormat.format(msgFormat, params);
    }

    public Object[] getRawParams() {
        return m_params;
    }

    public String getRawMessage() {
        return m_message;
    }
}
