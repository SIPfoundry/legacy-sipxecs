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
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.jivesoftware.util.cache.Cache;
import org.jivesoftware.util.cache.CacheFactory;

/**
 * Config doesn't use names as primary keys, whereas Openfire does. Thus, an update in config
 * equals a remove and a create in Openfire. Since for updates we don't receive the previous name
 * of the entity, we need to cache them.<br/>
 * This is more complicated for MUC rooms created for each configured conference, where we can't
 * retain the id either. We use two caches in this case.<br/>
 * CacheFactory will use a distributed cache when used in a clustered environment.
 */
public class CacheHolder {
    private static final Cache<String, String> USER_CACHE = CacheFactory.createCache("mongoUser");
    private static final Cache<String, Collection<String>> USER_GROUP_CACHE = CacheFactory.createCache("mongoGroupUser");
    private static final Cache<String, String> GROUP_CACHE = CacheFactory.createCache("mongoGroup");
    private static final Cache<Long, String> MUC_ROOM_CACHE = CacheFactory.createCache("mongoMucRoom");

    public static void putUser(String id, String name) {
        USER_CACHE.put(id, name);
    }

    public static String getUserName(String id) {
        return USER_CACHE.get(id);
    }

    public static void removeUser(String id) {
        USER_CACHE.remove(id);
    }
    
    public static void putUserGroups(String id, Collection<String> groups) {
        USER_GROUP_CACHE.put(id, groups);
    }
    
    public static Collection<String> getUserGroups(String id) {
        return USER_GROUP_CACHE.get(id);
    }
    
    public static void removeUserGroups(String id) {
        USER_GROUP_CACHE.remove(id);
    }

    public static void removeUserByName(String name) {
        if (name != null) {
            String id = null;
            for (Map.Entry<String, String> entry : USER_CACHE.entrySet()) {
                if (name.equals(entry.getValue())) {
                    id = entry.getKey();
                    break;
                }
            }
            USER_CACHE.remove(id);
        }
    }

    public static void putGroup(String id, String name) {
        GROUP_CACHE.put(id, name);
    }

    public static String getGroupName(String id) {
        return GROUP_CACHE.get(id);
    }

    public static void removeGroup(String id) {
        GROUP_CACHE.remove(id);
    }

    public static void removeGroupByName(String name) {
        if (name != null) {
            Set<String> ids = new HashSet<String>();
            for (Map.Entry<String, String> entry : GROUP_CACHE.entrySet()) {
                if (name.equals(entry.getValue())) {
                    ids.add(entry.getKey());
                }
            }
            for (String id : ids) {
                GROUP_CACHE.remove(id);
            }
        }
    }

    public static void putRoom(Long id, String name) {
        MUC_ROOM_CACHE.put(id, name);
    }

    public static String getRoomName(Long id) {
        return MUC_ROOM_CACHE.get(id);
    }

    public static void removeRoom(Long id) {
        MUC_ROOM_CACHE.remove(id);
    }
}