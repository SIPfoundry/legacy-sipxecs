/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;

public abstract class SbcDeviceDeleteListener implements DaoEventListener {

    public void onDelete(Object entity) {
        if (entity instanceof SbcDevice) {
            onSbcDeviceDelete((SbcDevice) entity);
        }
    }

    public void onSave(Object entity) {
    }

    protected abstract void onSbcDeviceDelete(SbcDevice sbcDevice);
}
