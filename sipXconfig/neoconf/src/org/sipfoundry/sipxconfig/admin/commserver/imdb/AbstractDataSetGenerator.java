/*
 *
 *
 * Copyright (C) 2011 eZuce inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
