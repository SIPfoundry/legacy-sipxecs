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

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;

public class TargetSqueezeAdapter implements IPrimaryKeyConverter {

    public Object getPrimaryKey(Object value) {
        MRTGTarget target = (MRTGTarget) value;
        return target.getTitle();
    }

    public Object getValue(Object primaryKey) {
        MRTGTarget target = new MRTGTarget();
        target.setTitle(primaryKey.toString());
        return target;
    }
}
