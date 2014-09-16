/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.provider;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.OfflineMessage;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.provider.OfflineMessageProvider;
import org.jivesoftware.util.FastDateFormat;
import org.jivesoftware.util.StringUtils;
import org.jivesoftware.util.XMPPDateTimeFormat;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.WriteResult;

public class MongoOfflineMessageProvider extends BaseMongoProvider implements OfflineMessageProvider {
    private static final String COLLECTION_NAME = "ofOffline";

    private static final FastDateFormat DATE_FORMAT = FastDateFormat.getInstance(
            XMPPDateTimeFormat.XMPP_DATETIME_FORMAT, TimeZone.getTimeZone("UTC"));
    private static final FastDateFormat OLD_DATE_FORMAT = FastDateFormat.getInstance(
            XMPPDateTimeFormat.XMPP_DELAY_DATETIME_FORMAT, TimeZone.getTimeZone("UTC"));
    private static Logger log = Logger.getLogger(MongoOfflineMessageProvider.class);

    /**
     * Pattern to use for detecting invalid XML characters. Invalid XML characters will be removed
     * from the stored offline messages.
     */
    private static final Pattern PATTERN = Pattern.compile("&\\#[\\d]+;");

    public MongoOfflineMessageProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection offlineCollection = getDefaultCollection();

        offlineCollection.ensureIndex("username");

        DBObject index = new BasicDBObject();

        index.put("username", 1);
        index.put("messageID", 1);
        offlineCollection.ensureIndex(index);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void addMessage(String username, long messageID, String msgXML) {
        DBCollection offlineCollection = getDefaultCollection();

        DBObject toInsert = new BasicDBObject();

        toInsert.put("username", username);
        toInsert.put("messageID", messageID);
        toInsert.put("creationDate", StringUtils.dateToMillis(new java.util.Date()));
        toInsert.put("messageSize", msgXML.length());
        toInsert.put("stanza", msgXML);

        offlineCollection.insert(toInsert);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean deleteMessage(String username, Date creationDate) {
        DBCollection offlineCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();
        toDelete.put("username", username);
        toDelete.put("creationDate", StringUtils.dateToMillis(creationDate));
        WriteResult result = offlineCollection.remove(toDelete);

        return !org.apache.commons.lang.StringUtils.isEmpty(result.getError());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean deleteMessages(String username) {
        DBCollection offlineCollection = getDefaultCollection();

        DBObject toDelete = new BasicDBObject();
        toDelete.put("username", username);
        WriteResult result = offlineCollection.remove(toDelete);

        return !org.apache.commons.lang.StringUtils.isEmpty(result.getError());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public OfflineMessage getMessage(String username, Date creationDate, SAXReader xmlReader) {
        DBCollection offlineCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        query.put("creationDate", StringUtils.dateToMillis(creationDate));
        DBObject keys = new BasicDBObject();
        keys.put("stanza", 1);

        DBObject dbObj = offlineCollection.findOne(query);
        String msgXml = (String) dbObj.get("stanza");

        return fromString(msgXml, creationDate, xmlReader);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<OfflineMessage> getMessages(String username, boolean delete, SAXReader xmlReader) {
        List<OfflineMessage> messages = new ArrayList<OfflineMessage>();
        DBCollection offlineCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();
        query.put("username", username);
        DBObject keys = new BasicDBObject();
        keys.put("stanza", 1);
        keys.put("creationDate", 1);

        for (DBObject dbObj : offlineCollection.find(query, keys)) {
            String msgXml = (String) dbObj.get("stanza");
            Date creationDate = new Date(Long.parseLong((String) dbObj.get("creationDate")));
            messages.add(fromString(msgXml, creationDate, xmlReader));
        }

        // Check if the offline messages loaded should be deleted, and that there are
        // messages to delete
        if (delete && !messages.isEmpty()) {
            log.debug("deleting offline messages for user " + username);
            offlineCollection.remove(query);
        }

        return messages;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getSize() {
        return getSize(new BasicDBObject());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getSize(String username) {
        DBObject query = new BasicDBObject();

        query.put("username", username);

        return getSize(query);
    }

    private int getSize(DBObject toFind) {
        DBCollection offlineCollection = getDefaultCollection();

        int totalSize = 0;
        BasicDBObject keys = new BasicDBObject();
        keys.put("messageSize", 1);

        for (DBObject dbObj : offlineCollection.find(toFind, keys)) {
            int messageSize = (Integer) dbObj.get("messageSize");
            totalSize += messageSize;
        }

        return totalSize;
    }

    private static OfflineMessage fromString(String msgXml, Date creationDate, SAXReader xmlReader) {
        OfflineMessage message = null;
        try {
            try {
                message = new OfflineMessage(creationDate, xmlReader.read(new StringReader(msgXml)).getRootElement());
            } catch (DocumentException e) {
                // Try again after removing invalid XML chars (e.g. &#12;)
                Matcher matcher = PATTERN.matcher(msgXml);
                if (matcher.find()) {
                    String invalidRemoved = matcher.replaceAll("");
                    Document doc = xmlReader.read(new StringReader(invalidRemoved));
                    message = new OfflineMessage(creationDate, doc.getRootElement());
                }
            }

            if (message != null) {
                // Add a delayed delivery (XEP-0203) element to the message.
                Element delay = message.addChildElement("delay", "urn:xmpp:delay");
                delay.addAttribute("from", XMPPServer.getInstance().getServerInfo().getXMPPDomain());
                delay.addAttribute("stamp", DATE_FORMAT.format(creationDate));
                // Add a legacy delayed delivery (XEP-0091) element to the
                // message. XEP is obsolete and support should be dropped in
                // future.
                delay = message.addChildElement("x", "jabber:x:delay");
                delay.addAttribute("from", XMPPServer.getInstance().getServerInfo().getXMPPDomain());
                delay.addAttribute("stamp", OLD_DATE_FORMAT.format(creationDate));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return message;
    }
}
