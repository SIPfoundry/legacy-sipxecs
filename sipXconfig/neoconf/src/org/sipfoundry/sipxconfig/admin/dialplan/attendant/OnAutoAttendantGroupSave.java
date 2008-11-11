/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.event.EntitySaveListener;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContextImpl.ATTENDANT_GROUP_ID;

/**
 * OnAutoAttendantGroupSave regenerates all auto-attendants whenever group that stores defaults
 * changes.
 */
class OnAutoAttendantGroupSave extends EntitySaveListener<Group> {
    private DialPlanContext m_dialPlanContext;

    private AutoAttendantsConfig m_autoAttendantsConfig;

    public OnAutoAttendantGroupSave() {
        super(Group.class);
    }

    @Override
    protected void onEntitySave(Group group) {
        if (!ATTENDANT_GROUP_ID.equals(group.getResource())) {
            // only interested in attendant group changes here
            return;
        }
        if (group.isNew()) {
            // ignore newly saved group
            return;
        }
        m_autoAttendantsConfig.generate(m_dialPlanContext);
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setAutoAttendantsConfig(AutoAttendantsConfig autoAttendantsConfig) {
        m_autoAttendantsConfig = autoAttendantsConfig;
    }
}
