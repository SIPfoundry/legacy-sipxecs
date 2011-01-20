/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.BasicDBObject;
import org.sipfoundry.sipxconfig.common.Replicable;

public abstract class DataSetRecord extends BasicDBObject {
    private Replicable m_entity;

    public Replicable getEntity() {
        return m_entity;
    }

    public void setEntity(Replicable entity) {
        m_entity = entity;
    }
}
