/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBCollection;
import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.User;

public abstract class AbstractDataSetGenerator {
    private DBCollection m_dbCollection;
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected CoreContext getCoreContext() {
        return m_coreContext;
    }

    /**
     * @return SIP domain - if not set uses m_coreContext to retrieve domain
     */
    protected String getSipDomain() {
        return m_coreContext.getDomainName();
    }

    public DBCollection getDbCollection() {
        return m_dbCollection;
    }

    public void setDbCollection(DBCollection dbCollection) {
        m_dbCollection = dbCollection;
    }

    public abstract void generate(Replicable entity, DBObject top);

    protected abstract DataSet getType();

    public static String getEntityId(Replicable entity) {
        String id = "";
        if (entity instanceof BeanWithId) {
            id = entity.getClass().getSimpleName() + ((BeanWithId) entity).getId();
        }
        if (entity instanceof SpecialUser) {
            id = ((SpecialUser) entity).getUserName();
        } else if (entity instanceof User) {
            User u = (User) entity;
            if (u.isNew()) {
                id = u.getUserName();
            }
        } else if (entity instanceof ExternalAlias) {
            ExternalAlias alias = (ExternalAlias) entity;
            id = alias.getName();
        }
        return id;
    }
}
