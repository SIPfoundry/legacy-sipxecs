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
 * If you want the message to be localized on UI layer do this:
 * 
 * throw new UserException(false, "msg.key", "bongo", 5);
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

    /** unlocalized version of the message */
    private String m_key;
    private Object[] m_params = ArrayUtils.EMPTY_OBJECT_ARRAY;

    public UserException() {
    }

    public UserException(Throwable cause) {
        super(cause);
    }

    /**
     * Shorthand version if message is localized.
     */
    public UserException(String message, Object... params) {
        this(true, message, params);
    }

    /**
     * @param localized - true if message is already localized, false if it's just a key
     * @param message - message format (does not have to have any parameters)
     * @param params - parameters to be passed to MessageFormat when displaying exception errror
     */
    public UserException(boolean localized, String message, Object... params) {
        if (localized) {
            m_message = message;
        } else {
            m_key = message;
        }
        m_message = message;
        m_params = params;
    }

    public String getMessage() {
        if (m_message != null) {
            return format(m_message);
        }
        if (getCause() != null) {
            return getCause().getLocalizedMessage();
        }
        return StringUtils.EMPTY;
    }

    public String getKey() {
        return m_key;
    }

    public String format(String msgFormat) {
        return MessageFormat.format(msgFormat, m_params);
    }
}
