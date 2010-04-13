/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class AttendantSpecialMode extends BeanWithId {
    private boolean m_enabled;
    private AutoAttendant m_attendant;

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void setAttendant(AutoAttendant attendant) {
        m_attendant = attendant;
    }

    public AutoAttendant getAttendant() {
        return m_attendant;
    }
}
