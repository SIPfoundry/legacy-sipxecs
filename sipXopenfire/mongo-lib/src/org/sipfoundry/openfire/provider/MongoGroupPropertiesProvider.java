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

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

import org.jivesoftware.openfire.group.DefaultGroupPropertyMap;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupAlreadyExistsException;
import org.jivesoftware.openfire.provider.GroupPropertiesProvider;
import org.jivesoftware.util.PersistableMap;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

public class MongoGroupPropertiesProvider extends BaseMongoProvider implements GroupPropertiesProvider {
    private static final String COLLECTION_NAME = "ofGroupProp";

    // GPN = group property name
    private static final String GPN_DISPLAY_NAME = "sharedRoster.displayName";
    private static final String GPN_SHOW_IN_ROSTER = "sharedRoster.showInRoster";

    public MongoGroupPropertiesProvider() {
        setDefaultCollectionName(COLLECTION_NAME);
        DBCollection grpPropsCollection = getDefaultCollection();
        DBObject index = new BasicDBObject();

        index.put("groupname", 1);
        index.put("name", 1);
        grpPropsCollection.ensureIndex(index);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PersistableMap<String, String> loadProperties(Group group) {
        PersistableMap<String, String> grpProps = new DefaultGroupPropertyMap<String, String>(group);

        DBCollection grpPropsCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("groupname", group.getName());

        for (DBObject grpPropsObj : grpPropsCollection.find(query)) {
            String propName = (String) grpPropsObj.get("name");
            String propValue = (String) grpPropsObj.get("propValue");
            grpProps.put(propName, propValue, false);
        }

        // if missing, add properties without persisting and handle persistence separately
        // trying to add with persistence will trigger in infinite recursion
        if (grpProps.get(GPN_DISPLAY_NAME) == null) {
            grpProps.put(GPN_DISPLAY_NAME, group.getName(), false);
            insertProperty(group.getName(), GPN_DISPLAY_NAME, group.getName());
        }
        if (grpProps.get(GPN_SHOW_IN_ROSTER) == null) {
            grpProps.put(GPN_SHOW_IN_ROSTER, "onlyGroup", false);
            insertProperty(group.getName(), GPN_SHOW_IN_ROSTER, "onlyGroup");
        }

        return grpProps;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void insertProperty(String groupName, String propName, String propValue) {
        DBCollection grpPropsCollection = getDefaultCollection();

        DBObject toInsert = new BasicDBObject();

        toInsert.put("groupname", groupName);
        toInsert.put("name", propName);
        toInsert.put("propValue", propValue);

        grpPropsCollection.insert(toInsert);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void updateProperty(String groupName, String propName, String propValue) {
        // nothing to do
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void deleteProperty(String groupName, String propName) {
        // nothing to do
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean deleteGroupProperties(String groupName) {
        // nothing to do

        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<String> getSharedGroupsNames() {
        throw new UnsupportedOperationException("Not implemented");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<String> getPublicSharedGroupNames() {
        return search(GPN_SHOW_IN_ROSTER, "everybody");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<String> getVisibleGroupNames(String userGroup) {
        DBCollection grpPropsCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("name", "sharedRoster.groupList");
        query.put("propValue", Pattern.compile("\\.*" + userGroup + "\\.*"));

        Set<String> names = new HashSet<String>();

        for (DBObject propObj : grpPropsCollection.find(query)) {
            names.add((String) propObj.get("groupName"));
        }

        return names;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean setName(String oldName, String newName) throws GroupAlreadyExistsException {
        // nothing to do

        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Collection<String> search(String key, String value) {
        DBCollection grpPropsCollection = getDefaultCollection();

        DBObject query = new BasicDBObject();

        query.put("name", key);
        query.put("propValue", value);

        Set<String> names = new HashSet<String>();

        for (DBObject propObj : grpPropsCollection.find(query)) {
            names.add((String) propObj.get("groupName"));
        }

        return names;
    }
}
