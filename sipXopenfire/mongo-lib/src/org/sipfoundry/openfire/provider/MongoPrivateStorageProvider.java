/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Map;

import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.provider.PrivateStorageProvider;
import org.jivesoftware.openfire.user.User;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoPrivateStorageProvider extends BaseMongoProvider implements PrivateStorageProvider {
    private static final Logger log = Logger.getLogger(MongoPrivateStorageProvider.class);

    private static final String COLLECTION_NAME = "ofPrivate";

    public MongoPrivateStorageProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection prvStorageCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("username", 1);
        index.put("namespace", 1);
        index.put("name", 1);

        prvStorageCollection.ensureIndex(index);
    }

    @Override
    public void add(String username, Element data) {
        log.debug(String.format("Writing private data for %s", username));
        try {
            StringWriter writer = new StringWriter();
            data.write(writer);
            log.debug(String.format("Writing private data %s", writer.toString()));

            DBCollection prvStorageCollection = getDefaultCollection();
            DBObject query = new BasicDBObject();

            query.put("username", username);
            query.put("namespace", data.getNamespaceURI());
            DBObject existing = prvStorageCollection.findOne(query);

            if (existing == null) {
                log.debug("new data");
                DBObject toInsert = new BasicDBObject();

                toInsert.put("username", username);
                toInsert.put("namespace", data.getNamespaceURI());
                toInsert.put("name", data.getName());
                toInsert.put("privateData", writer.toString());

                prvStorageCollection.insert(toInsert);
            } else {
                log.debug("existing data");
                existing.put("name", data.getName());
                existing.put("privateData", writer.toString());

                prvStorageCollection.save(existing);
            }
        } catch (IOException e) {
            log.error("Error storing data: " + e.getMessage());
        }
    }

    @Override
    public Element get(String username, Element data, SAXReader reader) {
        DBCollection prvStorageCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();
        Element result = data;

        query.put("username", username);
        query.put("namespace", data.getNamespaceURI());
        log.debug(String.format("Retrieving data for user %s and namespace %s", username, data.getNamespaceURI()));
        DBObject existing = prvStorageCollection.findOne(query);

        if (existing != null) {
            String prvData = ((String) existing.get("privateData")).trim();

            try {
                Document doc = reader.read(new StringReader(prvData));
                result = doc.getRootElement();
            } catch (DocumentException e) {
                log.error("Error retrieving data: " + e.getMessage());
            }
        }
        log.debug(String.format("Found data: %s", result.asXML()));

        return result;
    }

    @Override
    public void userDeleting(User user, Map<String, Object> params) {
        DBCollection prvStorageCollection = getDefaultCollection();
        DBObject toDelete = new BasicDBObject();

        toDelete.put("username", user.getUsername());

        prvStorageCollection.remove(toDelete);

    }
}
