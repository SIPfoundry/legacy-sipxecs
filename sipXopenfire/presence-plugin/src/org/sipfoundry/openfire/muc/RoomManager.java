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
package org.sipfoundry.openfire.muc;

import java.util.Collection;

import org.apache.log4j.Logger;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.Namespace;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.muc.ConflictException;
import org.jivesoftware.openfire.muc.ForbiddenException;
import org.jivesoftware.openfire.muc.HistoryStrategy;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.muc.MultiUserChatService;
import org.jivesoftware.openfire.muc.NotAllowedException;
import org.jivesoftware.openfire.muc.cluster.UpdateHistoryStrategy;
import org.jivesoftware.openfire.provider.PrivateStorageProvider;
import org.jivesoftware.openfire.provider.ProviderFactory;
import org.jivesoftware.util.AlreadyExistsException;
import org.sipfoundry.openfire.plugin.presence.SipXBookmarkManager;
import org.xmpp.packet.JID;

public class RoomManager {
    private static final Logger logger = Logger.getLogger(RoomManager.class);

    private static final String MUC_SUBDOMAIN = "conference";
    private static final String DEFAULT_DESCRIPTION = "default MUC service";
    private static final String STORAGE_TAG = "storage";
    private static final Namespace NAMESPACE = DocumentHelper.createNamespace("", "storage:bookmarks");
    private static final String BOOKMARK_TAG = "conference";
    private static final String BOOKMARK_NAME = "name";
    private static final String BOOKMARK_AUTOJOIN = "autojoin";
    private static final String BOOKMARK_JID = "jid";

    private static final PrivateStorageProvider PROVIDER = ProviderFactory.getPrivateStorageProvider();

    public static MultiUserChatService createChatRoomService(String subdomain) {
        MultiUserChatService mucService = XMPPServer.getInstance().getMultiUserChatManager()
                .getMultiUserChatService(subdomain);
        if (mucService == null) {
            try {
                mucService = XMPPServer.getInstance().getMultiUserChatManager()
                        .createMultiUserChatService(subdomain, "default MUC service", false);
                Collection<JID> admins = XMPPServer.getInstance().getAdmins();
                JID admin = admins.iterator().next();
                mucService.addSysadmin(admin);
                mucService.setLogConversationsTimeout(60);
                mucService.setLogConversationBatchSize(100);
                HistoryStrategy historyStrategy = new HistoryStrategy(null);
                historyStrategy.setType(HistoryStrategy.Type.none);
                new UpdateHistoryStrategy(subdomain, historyStrategy).run();
                mucService.enableService(true, true);
                mucService.setRoomCreationRestricted(false);
            } catch (Exception ex) {
                logger.error("createChatRoomService caught " + ex);
            }
        }

        return mucService;
    }

    public static MUCRoom createAndConfigureRoom(String name, String description, String confOwner,
            boolean moderated, boolean publicRoom, boolean membersOnly, String pin) {
        MultiUserChatService mucService = XMPPServer.getInstance().getMultiUserChatManager()
                .getMultiUserChatService(MUC_SUBDOMAIN);

        if (mucService == null) {
            try {
                Collection<JID> admins = XMPPServer.getInstance().getAdmins();
                JID admin = admins.iterator().next();
                mucService = XMPPServer.getInstance().getMultiUserChatManager()
                        .createMultiUserChatService(MUC_SUBDOMAIN, DEFAULT_DESCRIPTION, false);
                mucService.addSysadmin(admin);
                mucService.setLogConversationsTimeout(60);
                mucService.setLogConversationBatchSize(100);
                HistoryStrategy historyStrategy = new HistoryStrategy(null);
                historyStrategy.setType(HistoryStrategy.Type.none);
                new UpdateHistoryStrategy(MUC_SUBDOMAIN, historyStrategy).run();
                mucService.enableService(true, true);
                mucService.setRoomCreationRestricted(false);
            } catch (AlreadyExistsException ex) {
                // shouldn't happen since we've just looked and it wasn't there
                logger.error("createChatRoomService caught " + ex);
            }
        }

        MUCRoom mucRoom = null;
        if (mucService != null) {
            try {
                JID jid = new JID(confOwner);
                mucRoom = mucService.getChatRoom(name, jid);

                mucRoom.setNaturalLanguageName(name);

                // update bookmark
                ensureBookmark(mucRoom, jid);

                mucRoom.unlock(mucRoom.getRole());

                mucRoom.setPersistent(true);

                // add new owner and remove all others.
                // Note: cannot remove all first then add as this throws ConflictException
                if (!mucRoom.getOwners().contains(jid)) {
                    mucRoom.addOwner(jid, mucRoom.getRole());
                }
                for (JID formerOwner : mucRoom.getOwners()) {
                    if (!formerOwner.equals(jid)) {
                        mucRoom.addNone(formerOwner, mucRoom.getRole());
                    }
                }

                for (JID admins : XMPPServer.getInstance().getAdmins()) {
                    if (!mucRoom.getOwners().contains(admins)) {
                        mucRoom.addOwner(jid, mucRoom.getRole());
                    }
                }

                mucRoom.setCanAnyoneDiscoverJID(true);
                mucRoom.setChangeNickname(true);
                mucRoom.setModerated(moderated);
                mucRoom.setMembersOnly(membersOnly);
                mucRoom.setRegistrationEnabled(true);
                mucRoom.setPublicRoom(publicRoom);
                mucRoom.setCanOccupantsInvite(false);
                mucRoom.setDescription(description != null ? description : "");
                mucRoom.setPassword(pin);
                mucRoom.setCanOccupantsChangeSubject(true);
                mucRoom.setChangeNickname(true);
                mucRoom.setLogEnabled(false);
                mucRoom.saveToDB();
            } catch (NotAllowedException e) {
                logger.warn(String.format("Error creating room %s. %s", name, e.getMessage()));
            } catch (ForbiddenException e) {
                logger.warn(String.format("User %s tried to perform a forbidden operation.", confOwner), e);
            } catch (ConflictException e) {
                logger.warn("Error trying to modify MUC room.", e);
            }
        }

        return mucRoom;
    }

    /**
     * When a room is created we create a bookmark for the owner of this room
     *
     * @param mucRoom
     * @param jid
     */
    private static void ensureBookmark(MUCRoom mucRoom, JID jid) {
        if (SipXBookmarkManager.isInitialized()) {
            SipXBookmarkManager bookmarkManager = SipXBookmarkManager.getInstance();
            bookmarkManager.createMUCBookmark(jid.getNode(), mucRoom.getName(), mucRoom.getJID().toBareJID());
        }
    }

    public static Element getBookmarks(String owner) {
        Element bookmarks = DocumentHelper.createElement(STORAGE_TAG);
        bookmarks.add(NAMESPACE);

        return PROVIDER.get(owner, bookmarks, new SAXReader());
    }

    public static Element buildBookmarkElement(String bookmarkName, String jid) {
        Element conference = DocumentHelper.createElement(BOOKMARK_TAG);

        DocumentHelper.createAttribute(conference, BOOKMARK_NAME, bookmarkName);
        DocumentHelper.createAttribute(conference, BOOKMARK_AUTOJOIN, "false");
        DocumentHelper.createAttribute(conference, BOOKMARK_JID, jid);

        return conference;
    }
}
