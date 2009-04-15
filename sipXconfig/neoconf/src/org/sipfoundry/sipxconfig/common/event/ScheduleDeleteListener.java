/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;

public abstract class ScheduleDeleteListener implements DaoEventListener {

    public void onDelete(Object entity) {
        if (entity instanceof Schedule) {
            onScheduleDelete((Schedule) entity);
        }
    }

    public void onSave(Object entity_) {
    }

    protected abstract void onScheduleDelete(Schedule schedule);
}
