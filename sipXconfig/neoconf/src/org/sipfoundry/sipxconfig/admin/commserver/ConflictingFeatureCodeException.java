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

public class ConflictingFeatureCodeException extends UserException {
    private Setting m_a;
    private Setting m_b;
    ConflictingFeatureCodeException(Setting a, Setting b) {
        super("Conflicting feature codes");
        m_a = a;
        m_b = b;
    }

    @Override
    public String getMessage() {
        Object[] params = new String[] {
                m_a.getMessageSource().getMessage(m_a.getLabelKey(), null, Locale.getDefault()),
                m_b.getMessageSource().getMessage(m_b.getLabelKey(), null, Locale.getDefault())
        };
        return String.format("Conflicting feature codes: %s and %s", params);
    }
}
