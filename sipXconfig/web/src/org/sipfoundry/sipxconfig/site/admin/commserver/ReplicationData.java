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

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class ReplicationData extends PageWithCallback {

    public static final String PAGE = "admin/commserver/ReplicationData";

    public abstract SipxReplicationContext getSipxReplicationContext();

    public abstract String getDataSetName();

    public abstract void setDataSetName(String name);

    public String getXml() {
        return getSipxReplicationContext().getXml(DataSet.getEnum(getDataSetName()));
    }
}
