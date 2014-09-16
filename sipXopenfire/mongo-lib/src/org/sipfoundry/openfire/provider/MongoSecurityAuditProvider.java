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

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.jivesoftware.database.SequenceManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.provider.SecurityAuditProvider;
import org.jivesoftware.openfire.security.EventNotFoundException;
import org.jivesoftware.openfire.security.SecurityAuditEvent;
import org.jivesoftware.util.JiveConstants;
import org.jivesoftware.util.StringUtils;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoSecurityAuditProvider extends BaseMongoProvider implements SecurityAuditProvider {
    private static final String COLLECTION_NAME = "ofSecurityAuditLog";

    public MongoSecurityAuditProvider() {
        setDefaultCollectionName(COLLECTION_NAME);

        DBCollection saLogCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("msgID", 1);
        saLogCollection.ensureIndex(index);
    }

    @Override
    public void logEvent(String username, String summary, String details) {
        DBCollection saLogCollection = getDefaultCollection();
        DBObject toInsert = new BasicDBObject();
        long msgID = SequenceManager.nextID(JiveConstants.SECURITY_AUDIT);

        toInsert.put("msgID", msgID);
        toInsert.put("username", username);
        toInsert.put("entryStamp", new Date().getTime());
        toInsert.put("summary", StringUtils.abbreviate(summary, 250));
        toInsert.put("node", XMPPServer.getInstance().getServerInfo().getHostname());
        toInsert.put("details", details);

        saLogCollection.insert(toInsert);
    }

    @Override
    public List<SecurityAuditEvent> getEvents(String username, Integer skipEvents, Integer numEvents,
            Date startTime, Date endTime) {
        List<SecurityAuditEvent> events = new ArrayList<SecurityAuditEvent>();
        DBCollection saLogCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();

        if (username != null) {
            query.put("username", username);
        }
        if (startTime != null) {
            query.put("entryStamp", username);
        }
        if (endTime != null) {
            query.put("entryStamp", username);
        }

        int skip = skipEvents != null ? skipEvents : 0;
        int limit = numEvents != null ? numEvents : Integer.MAX_VALUE;

        for (DBObject evtObj : saLogCollection.find(query).skip(skip).limit(limit)) {
            SecurityAuditEvent event = new SecurityAuditEvent();
            event.setMsgID((Long) evtObj.get("msgID"));
            event.setUsername((String) evtObj.get("username"));
            event.setEventStamp(new Date((Long) evtObj.get("entryStamp")));
            event.setSummary((String) evtObj.get("summary"));
            event.setNode((String) evtObj.get("node"));
            event.setDetails((String) evtObj.get("details"));
            events.add(event);
        }

        return events;
    }

    @Override
    public SecurityAuditEvent getEvent(Integer msgID) throws EventNotFoundException {
        SecurityAuditEvent event = null;
        DBCollection saLogCollection = getDefaultCollection();
        DBObject query = new BasicDBObject();

        query.put("msgID", msgID);
        DBObject evtObj = saLogCollection.findOne(query);

        if (evtObj != null) {
            event = new SecurityAuditEvent();
            event.setMsgID(msgID);
            event.setUsername((String) evtObj.get("username"));
            event.setEventStamp(new Date((Long) evtObj.get("entryStamp")));
            event.setSummary((String) evtObj.get("summary"));
            event.setNode((String) evtObj.get("node"));
            event.setDetails((String) evtObj.get("details"));
        }

        return event;
    }

    @Override
    public Integer getEventCount() {
        return (int) getDefaultCollection().count();
    }

    @Override
    public boolean isWriteOnly() {
        return false;
    }

    @Override
    public String getAuditURL() {
        return null;
    }

    @Override
    public boolean blockUserEvents() {
        return false;
    }

    @Override
    public boolean blockGroupEvents() {
        return false;
    }

}
