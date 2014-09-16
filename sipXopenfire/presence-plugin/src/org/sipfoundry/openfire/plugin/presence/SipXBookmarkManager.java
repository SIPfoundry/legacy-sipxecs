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
package org.sipfoundry.openfire.plugin.presence;

import java.util.List;

import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.XPath;
import org.dom4j.xpath.DefaultXPath;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.provider.PrivateStorageProvider;
import org.jivesoftware.openfire.provider.ProviderFactory;
import org.sipfoundry.openfire.muc.RoomManager;

public class SipXBookmarkManager {
    private static final Logger log = Logger.getLogger(SipXBookmarkManager.class);

    private static SipXBookmarkManager instance;

    private final PrivateStorageProvider provider = ProviderFactory.getPrivateStorageProvider();

    private static final String CONFERENCE_DOMAIN = "conference"
            + XMPPServer.getInstance().getServerInfo().getXMPPDomain();

    private SipXBookmarkManager() {
    }

    public static synchronized void initialize() {
        if (null == instance) {
            try {
                instance = new SipXBookmarkManager();
            } catch (Exception ex) {
                log.error("Cannot create manager ", ex);
            }
        }
    }

    public static SipXBookmarkManager getInstance() {
        if (instance == null) {
            log.error("Bookmark Manager is not initialized");
        }
        return instance;
    }

    public static boolean isInitialized() {
        return instance != null;
    }

    public void createMUCBookmark(String owner, String bookmarkName, String bookmarkJid) {
        Element currentRooms = RoomManager.getBookmarks(owner);
        XPath roomPath = new DefaultXPath("conference[@name='" + bookmarkName + "'|jid='" + bookmarkJid + "']");
        Node existingRoom = roomPath.selectSingleNode(currentRooms);

        if (existingRoom == null) {
            Element newRoom = RoomManager.buildBookmarkElement(bookmarkName, bookmarkJid);

            currentRooms.add(newRoom);
            provider.add(owner, currentRooms);
        } else {
            log.debug(String.format("Bookmark %s[%s] for user %s already exists", bookmarkName, bookmarkJid, owner));
        }
    }

    /**
     * Deletes a bookmark being given the room name Make sure that SipXopenfire plugin class
     * creates the singleton during initialization
     *
     * @param roomName
     */
    public void deleteMUCBookmark(String owner, String roomName) {
        Element currentRooms = RoomManager.getBookmarks(owner);
        XPath roomPath = new DefaultXPath("conference[@jid='" + roomName + "@" + CONFERENCE_DOMAIN + "']");
        @SuppressWarnings("unchecked")
        List<Node> matches = roomPath.selectNodes(currentRooms);

        if (matches.size() > 0) {
            for (Node match : matches) {
                currentRooms.remove(match);
            }
            provider.add(owner, currentRooms);
        }
    }
}
