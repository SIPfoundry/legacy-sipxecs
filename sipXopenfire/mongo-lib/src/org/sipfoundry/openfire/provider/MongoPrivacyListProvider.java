/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import java.io.StringReader;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.privacy.PrivacyList;
import org.jivesoftware.openfire.provider.PrivacyListProvider;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoPrivacyListProvider extends BaseMongoProvider implements PrivacyListProvider {
    private static final Logger log = Logger.getLogger(MongoPrivacyListProvider.class);
    private static final String COLLECTION_NAME = "ofPrivacyList";

    private static final int POOL_SIZE = 50;
    private static final long POOL_TIMEOUT_SECONDS = 30;

    /**
     * Pool of SAX Readers. SAXReader is not thread safe so we need to have a pool of readers.
     */
    private final BlockingQueue<SAXReader> m_xmlReaders = new LinkedBlockingQueue<SAXReader>(POOL_SIZE);

    public MongoPrivacyListProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection prvListCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("username", 1);
        prvListCollection.ensureIndex(index);

        for (int i = 0; i < POOL_SIZE; i++) {
            SAXReader xmlReader = new SAXReader();
            xmlReader.setEncoding("UTF-8");
            m_xmlReaders.add(xmlReader);
        }

    }

    @Override
    public Map<String, Boolean> getPrivacyLists(String username) {
        Map<String, Boolean> privacyLists = new HashMap<String, Boolean>();
        DBCollection prvListCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        DBObject fields = new BasicDBObject();
        fields.put("name", 1);
        fields.put("isDefault", 1);

        for (DBObject dbObj : prvListCollection.find(query, fields)) {
            String name = (String) dbObj.get("name");
            Boolean isDefault = (Boolean) dbObj.get("isDefault");

            privacyLists.put(name, isDefault);
        }

        return privacyLists;
    }

    @Override
    public PrivacyList loadPrivacyList(String username, String listName) {
        PrivacyList privacyList = null;
        DBCollection prvListCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        query.put("name", listName);
        DBObject fields = new BasicDBObject();
        fields.put("list", 1);
        fields.put("isDefault", 1);
        DBObject grpPropsObj = prvListCollection.findOne(query, fields);

        if (grpPropsObj != null) {
            privacyList = buildPrivacyList(username, listName, grpPropsObj);
        }

        return privacyList;
    }

    @Override
    public PrivacyList loadDefaultPrivacyList(String username) {

        DBCollection prvListCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        query.put("isDefault", true);
        DBObject fields = new BasicDBObject();
        fields.put("list", 1);
        fields.put("name", 1);

        DBObject grpPropsObj = prvListCollection.findOne(query, fields);
        PrivacyList privacyList = null;

        if (grpPropsObj != null) {
            privacyList = buildPrivacyList(username, null, grpPropsObj);
        }

        return privacyList;
    }

    @Override
    public void createPrivacyList(String username, PrivacyList list) {
        DBCollection prvListCollection = getDefaultCollection();

        DBObject toInsert = new BasicDBObject();
        toInsert.put("username", username);
        toInsert.put("name", list.getName());
        toInsert.put("isDefault", list.isDefault());
        toInsert.put("list", list.asElement().asXML());

        prvListCollection.insert(toInsert);
    }

    @Override
    public void updatePrivacyList(String username, PrivacyList list) {
        DBCollection prvListCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        query.put("name", list.getName());

        DBObject update = new BasicDBObject();
        update.put("isDefault", list.isDefault());
        update.put("list", list.asElement().asXML());

        prvListCollection.findAndModify(query, new BasicDBObject("$set", update));
    }

    @Override
    public void deletePrivacyList(String username, String listName) {
        DBCollection prvListCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();
        toDelete.put("username", username);
        toDelete.put("name", listName);

        prvListCollection.insert(toDelete);
    }

    @Override
    public void deletePrivacyLists(String username) {
        DBCollection prvListCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();
        toDelete.put("username", username);

        prvListCollection.insert(toDelete);
    }

    private PrivacyList buildPrivacyList(String username, String listName, DBObject dbObj) {
        PrivacyList privacyList = null;
        String list = (String) dbObj.get("list");
        Boolean isDefault = (Boolean) dbObj.get("isDefault");
        if (isDefault == null) {
            isDefault = false;
        }

        SAXReader xmlReader = null;
        try {
            // Get a sax reader from the pool
            xmlReader = m_xmlReaders.poll(POOL_TIMEOUT_SECONDS, TimeUnit.SECONDS);
            Element listElement = xmlReader.read(new StringReader(list)).getRootElement();
            String actualListName = listName != null ? listName : (String) dbObj.get("name");
            log.debug("Creating privacy list for username=" + username + "; actualListName=" + actualListName + "; isDefault=" + isDefault + "; listElement=" + listElement);
            privacyList = new PrivacyList(username, actualListName, isDefault, listElement);
        } catch (Exception e) {
            log.error("Error reading privacy list", e);
        } finally {
            // Return the sax reader to the pool
            if (xmlReader != null) {
                m_xmlReaders.add(xmlReader);
            }
        }
        return privacyList;
    }
}
