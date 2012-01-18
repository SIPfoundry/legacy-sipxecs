/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

public class AttendantSpecialMode extends BeanWithId implements DeployConfigOnEdit {
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) DialPlanContext.FEATURE);
    }
}
