/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);
    private static final String DB_COLLECTION_NAME = "registrar";
    private static final String EXPIRED = "expired";
    private static final String IDENTITY = "identity";
    private static final String URI = "uri";
    private MongoTemplate m_nodedb;
    private DomainManager m_domainManager;

    /**
     * @see org.sipfoundry.sipxconfig.registrar.RegistrationContext#getRegistrations()
     */
    public List<RegistrationItem> getRegistrations() {
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(EXPIRED).is(Boolean.FALSE).get());
        return getItems(cursor);
    }

    public List<RegistrationItem> getRegistrationsByUser(User user) {
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(IDENTITY)
                .is(user.getIdentity(m_domainManager.getDomainName())).and(EXPIRED).is(Boolean.FALSE).get());
        return getItems(cursor);
    }

    private List<RegistrationItem> getItems(DBCursor cursor) {
        List<RegistrationItem> items = new ArrayList<RegistrationItem>(cursor.size());
        while (cursor.hasNext()) {
            DBObject registration = cursor.next();
            RegistrationItem item = new RegistrationItem();
            item.setContact((String) registration.get("contact"));
            item.setPrimary((String) registration.get("primary"));
            item.setExpires((Integer) registration.get("expirationTime"));
            item.setUri((String) registration.get(URI));
            item.setInstrument((String) registration.get("instrument"));
            item.setRegCallId((String) registration.get("callId"));
            item.setIdentity((String) registration.get(IDENTITY));
            items.add(item);
        }
        return items;
    }

    public MongoTemplate getNodedb() {
        return m_nodedb;
    }

    public void setNodedb(MongoTemplate nodedb) {
        m_nodedb = nodedb;
    }

    public void setDomainManager(DomainManager mgr) {
        m_domainManager = mgr;
    }
}
