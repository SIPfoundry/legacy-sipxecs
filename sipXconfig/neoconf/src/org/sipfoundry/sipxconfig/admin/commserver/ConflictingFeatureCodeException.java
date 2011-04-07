/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Locale;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.NoSuchMessageException;

public class ConflictingFeatureCodeException extends UserException {
    private String m_param1;
    private String m_param2;

    public ConflictingFeatureCodeException(String messageKey, Setting a, Setting b) {
        super(messageKey);
        try {
            m_param1 = a.getMessageSource().getMessage(a.getLabelKey(), null, Locale.getDefault());
            m_param2 = b.getMessageSource().getMessage(b.getLabelKey(), null, Locale.getDefault());
        } catch (NoSuchMessageException ex) {
            m_param1 = m_param1 == null ? a.getLabelKey() : m_param1;
            m_param2 = m_param2 == null ? b.getLabelKey() : m_param2;
        }
    }

    @Override
    public Object[] getRawParams() {
        return new Object[] {m_param1, m_param2};
    }
}
