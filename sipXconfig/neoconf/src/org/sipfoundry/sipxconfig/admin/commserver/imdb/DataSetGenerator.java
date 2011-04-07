/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.User;

public abstract class DataSetGenerator {
    public static final String ID = "id";
    public static final String IDENTITY = "ident";
    public static final String CONTACT = "cnt";
    public static final String UID = "uid";
    public static final String VALID_USER = "vld";
    public static final String CONF_EXT = "cnfext";
    public static final String CONF_NAME = "cnfnm";
    public static final String CONF_OWNER = "cnfown";
    public static final String CONF_PIN = "cnfpin";
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

    public abstract void generate(Replicable entity);

    protected abstract DataSet getType();

    public DBCollection getDbCollection() {
        return m_dbCollection;
    }

    public void setDbCollection(DBCollection dbCollection) {
        m_dbCollection = dbCollection;
    }

    // We can safely assume that every replicable entity is a beanwithid
    // We can treat special cases separately
    protected DBObject findOrCreate(Replicable entity) {
        DBCollection collection = getDbCollection();
        String id = getEntityId(entity);

        DBObject search = new BasicDBObject();
        search.put(ID, id);
        DBCursor cursor = collection.find(search);
        DBObject top = new BasicDBObject();
        if (!cursor.hasNext()) {
            top.put(ID, id);
        } else {
            top = cursor.next();
        }
        if (entity.getIdentity(getSipDomain()) != null) {
            top.put(IDENTITY, entity.getIdentity(getSipDomain()));
        }
        for (String key : entity.getMongoProperties(getSipDomain()).keySet()) {
            top.put(key, entity.getMongoProperties(getSipDomain()).get(key));
        }
        if (entity.isValidUser()) {
            top.put(VALID_USER, true);
        }
        return top;
    }

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
