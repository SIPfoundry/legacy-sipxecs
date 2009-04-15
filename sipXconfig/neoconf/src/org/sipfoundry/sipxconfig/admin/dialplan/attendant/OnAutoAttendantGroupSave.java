/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import org.sipfoundry.sipxconfig.common.event.EntitySaveListener;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager.ATTENDANT_GROUP_ID;

/**
 * OnAutoAttendantGroupSave regenerates all auto-attendants whenever group that stores defaults
 * changes.
 */
class OnAutoAttendantGroupSave extends EntitySaveListener<Group> {
    private ServiceConfigurator m_serviceConfigurator;

    private SipxIvrService m_sipxIvrService;

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
        m_serviceConfigurator.replicateServiceConfig(m_sipxIvrService, true);
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setSipxIvrService(SipxIvrService sipxIvrService) {
        m_sipxIvrService = sipxIvrService;
    }
}
