/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class ScheduledAttendant implements Cloneable {
    private boolean m_enabled;

    private AutoAttendant m_attendant;

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public AutoAttendant getAttendant() {
        return m_attendant;
    }

    public void setAttendant(AutoAttendant attendant) {
        m_attendant = attendant;
        setEnabled(attendant != null);
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    /**
     * Check if the attendant in question is referenced by this schedule
     *
     * @param attendant
     * @return true if any references have been found false otherwise
     */
    public boolean checkAttendant(AutoAttendant attendant) {
        if (m_attendant == null) {
            return false;
        }
        return m_attendant.equals(attendant);
    }
}
