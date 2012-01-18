/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

/*
 * this page is obsolete. MAYBE we can put here a page that shows all mongo database records.
 */
public abstract class ReplicationData extends PageWithCallback {

    public static final String PAGE = "admin/commserver/ReplicationData";

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract String getDataSetName();

    public abstract void setDataSetName(String name);

}
