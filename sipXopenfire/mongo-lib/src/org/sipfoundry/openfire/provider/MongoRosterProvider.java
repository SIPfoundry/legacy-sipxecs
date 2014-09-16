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
import java.util.Iterator;
import java.util.List;

import org.jivesoftware.database.SequenceManager;
import org.jivesoftware.openfire.provider.RosterItemProvider;
import org.jivesoftware.openfire.roster.RosterItem;
import org.jivesoftware.openfire.user.UserAlreadyExistsException;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.jivesoftware.util.JiveConstants;
import org.xmpp.packet.JID;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoRosterProvider extends BaseMongoProvider implements RosterItemProvider {
    private static final String COLLECTION_NAME = "ofRoster";

    public MongoRosterProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection rosterCollection = getDefaultCollection();

        rosterCollection.ensureIndex("rosterID");
        rosterCollection.ensureIndex("jid");
        rosterCollection.ensureIndex("username");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RosterItem createItem(String username, RosterItem item) throws UserAlreadyExistsException {
        DBCollection rosterCollection = getDefaultCollection();
        DBObject toInsert = new BasicDBObject();

        toInsert.put("rosterID", SequenceManager.nextID(JiveConstants.ROSTER));
        toInsert.put("username", username);
        toInsert.put("jid", item.getJid().toBareJID());
        toInsert.put("sub", item.getSubStatus().getValue());
        toInsert.put("ask", item.getAskStatus().getValue());
        toInsert.put("recv", item.getRecvStatus().getValue());
        toInsert.put("nick", item.getNickname());
        toInsert.put("groups", item.getGroups());

        rosterCollection.insert(toInsert);
        item.setID((Long) toInsert.get("rosterID"));

        return item;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteItem(String username, long rosterItemID) {
        DBCollection rosterGrpCollection = getCollection("ofRosterGroups");
        DBObject grpToRemove = new BasicDBObject();
        grpToRemove.put("rosterID", rosterItemID);

        rosterGrpCollection.remove(grpToRemove);

        DBCollection rosterCollection = getDefaultCollection();
        DBObject toRemove = new BasicDBObject();
        toRemove.put("rosterID", rosterItemID);

        rosterCollection.remove(toRemove);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getItemCount(String username) {
        DBCollection rosterCollection = getDefaultCollection();
        DBObject toQuery = new BasicDBObject();
        toQuery.put("username", username);

        return (int) rosterCollection.count(toQuery);

    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Iterator<RosterItem> getItems(String username) {
        List<RosterItem> items = new ArrayList<RosterItem>();

        DBCollection rosterCollection = getDefaultCollection();
        DBObject toQuery = new BasicDBObject();
        toQuery.put("username", username);

        for (DBObject row : rosterCollection.find(toQuery)) {
            long id = (Long) row.get("rosterID");
            JID jid = new JID((String) row.get("jid"));
            int subType = (Integer) row.get("sub");
            int askType = (Integer) row.get("ask");
            int recvType = (Integer) row.get("recv");
            String nick = (String) row.get("nick");

            RosterItem item = new RosterItem(id, jid, RosterItem.SubType.getTypeFromInt(subType),
                    RosterItem.AskType.getTypeFromInt(askType), RosterItem.RecvType.getTypeFromInt(recvType), nick,
                    null);

            items.add(item);
        }

        return items.iterator();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Iterator<String> getUsernames(String jid) {
        List<String> names = new ArrayList<String>();

        DBCollection rosterCollection = getDefaultCollection();
        DBObject toQuery = new BasicDBObject();
        toQuery.put("jid", jid);

        for (DBObject row : rosterCollection.find(toQuery)) {
            names.add((String) row.get("username"));
        }

        return names.iterator();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void updateItem(String username, RosterItem item) throws UserNotFoundException {
        DBCollection rosterCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("rosterID", item.getID());

        DBObject update = new BasicDBObject();

        update.put("sub", item.getSubStatus().getValue());
        update.put("ask", item.getAskStatus().getValue());
        update.put("recv", item.getRecvStatus().getValue());
        update.put("nick", item.getNickname());
        update.put("groups", item.getGroups());

        rosterCollection.findAndModify(query, new BasicDBObject("$set", update));
    }
}
