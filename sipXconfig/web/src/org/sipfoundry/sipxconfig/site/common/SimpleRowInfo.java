/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.components.RowInfo;

/**
 * Default implementation of information about a row the Table component needs to know to operate
 */
public class SimpleRowInfo implements RowInfo {
    public Object getSelectId(Object row) {
        if (row instanceof PrimaryKeySource) {
            PrimaryKeySource keySource = (PrimaryKeySource) row;
            return keySource.getPrimaryKey();
        }
        return row;
    }

    public boolean isSelectable(Object row) {
        return (row != null);
    }
}
