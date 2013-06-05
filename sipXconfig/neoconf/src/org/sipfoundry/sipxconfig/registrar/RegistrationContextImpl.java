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

import static org.sipfoundry.commons.mongo.MongoConstants.CALL_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.EXPIRATION_TIME;
import static org.sipfoundry.commons.mongo.MongoConstants.INSTRUMENT;
import static org.sipfoundry.commons.mongo.MongoConstants.PRIMARY;
import static org.sipfoundry.commons.mongo.MongoConstants.REG_CONTACT;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

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
    private static final String PATTERN_ALL = ".*";
    private MongoTemplate m_nodedb;
    private DomainManager m_domainManager;

    /**
     * @see org.sipfoundry.sipxconfig.registrar.RegistrationContext#getRegistrations()
     */
    @Override
    public List<RegistrationItem> getRegistrations() {
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(EXPIRED).is(Boolean.FALSE).get());
        return getItems(cursor);
    }

    @Override
    public List<RegistrationItem> getRegistrationsByUser(User user) {
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(IDENTITY)
                .is(user.getIdentity(m_domainManager.getDomainName())).and(EXPIRED).is(Boolean.FALSE).get());
        return getItems(cursor);
    }

    @Override
    public DBCursor getRegistrationsByLineId(String line) {
        Pattern linePattern = Pattern.compile("sip:" + line + "@.*");
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(URI).regex(linePattern).and(EXPIRED)
                .is(Boolean.FALSE).get());
        return cursor;
    }

    private List<RegistrationItem> getItems(DBCursor cursor) {
        List<RegistrationItem> items = new ArrayList<RegistrationItem>(cursor.size());
        while (cursor.hasNext()) {
            DBObject registration = cursor.next();
            RegistrationItem item = new RegistrationItem();
            item.setContact((String) registration.get(REG_CONTACT));
            item.setPrimary((String) registration.get(PRIMARY));
            item.setExpires((Integer) registration.get(EXPIRATION_TIME));
            item.setUri((String) registration.get(URI));
            item.setInstrument((String) registration.get(INSTRUMENT));
            item.setRegCallId((String) registration.get(CALL_ID));
            item.setIdentity((String) registration.get(IDENTITY));
            items.add(item);
        }
        return items;
    }

    @Override
    public DBCursor getRegistrationsByMac(String mac) {
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(INSTRUMENT).is(mac).and(EXPIRED)
                .is(Boolean.FALSE).get());
        return cursor;
    }

    @Override
    public DBCursor getRegistrationsByIp(String ip) {
        Pattern ipPattern = Pattern.compile(PATTERN_ALL + ip + PATTERN_ALL);
        DB datasetDb = m_nodedb.getDb();
        DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
        DBCursor cursor = registrarCollection.find(QueryBuilder.start(CALL_ID).regex(ipPattern).and(EXPIRED)
                .is(Boolean.FALSE).get());
        return cursor;
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
