/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

/**
 * Default SBC. Specifies Intranet routes, that are *not* sent trough the SBC.
 */
public class DefaultSbc extends Sbc {
    @Override
    public boolean onDeleteSbcDevice() {
        setSbcDevice(null);
        setEnabled(false);
        return false;
    }
}
