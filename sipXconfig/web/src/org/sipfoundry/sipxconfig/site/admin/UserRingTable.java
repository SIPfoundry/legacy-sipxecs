/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.common.CoreContext;

public abstract class UserRingTable extends BaseComponent {
    public abstract CoreContext getCoreContext();

    public abstract Collection getRowsToDelete();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();

    public abstract boolean getAddRow();
}
