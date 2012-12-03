/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.config;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.digester.Digester;
import org.apache.commons.lang.BooleanUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.provider.ProviderFactory;
import org.sipfoundry.commons.confdb.Conference;
import org.sipfoundry.commons.confdb.ConferenceService;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.UserGroup;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.sipfoundry.openfire.plugin.presence.SipXBookmarkManager;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;
import org.sipfoundry.openfire.plugin.presence.UserAccount;
import org.xmpp.packet.JID;

public class AccountsParser {
    private long lastModified;
    private final File watchFile;
    private static Logger logger = Logger.getLogger(AccountsParser.class);
    private XmppAccountInfo previousXmppAccountInfo = null;
    private final ConferenceService m_conferenceService;
    private final boolean m_parsingEnabled;

    private static Timer timer = new Timer();

    static {
        Logger digLogger = Logger.getLogger(Digester.class);
        digLogger.addAppender(new ConsoleAppender(new SimpleLayout()));
        digLogger.setLevel(Level.OFF);
        digLogger = Logger.getLogger("org.apache.commons.beanutils");
        digLogger.addAppender(new ConsoleAppender(new SimpleLayout()));
        digLogger.setLevel(Level.OFF);
    }

    class Scanner extends TimerTask {

        @Override
        public void run() {
            if (watchFile.lastModified() != AccountsParser.this.lastModified) {
                logger.info("XMPP account configuration changes detected - synchronize data");
                AccountsParser.this.lastModified = watchFile.lastModified();
                parseAccounts(m_conferenceService, m_parsingEnabled);
            }
        }
    }

    public void parseAccounts(ConferenceService conferenceService, boolean parseEnabled) {
        XmppAccountInfo newAccountInfo = AccountsParser.parse(conferenceService, parseEnabled);
        if (m_parsingEnabled) {
            logger.debug("Pruning unwanted users");
            pruneUnwantedXmppUsers( newAccountInfo.getXmppUserAccountNames() );
            logger.debug("Done pruning unwanted users");
            logger.debug("Pruning unwanted groups");
            pruneUnwantedXmppGroups( newAccountInfo.getXmppGroupNames() ); // prune groups before applying deltas - see XX-7886
            logger.debug("Done pruning unwanted groups");
            logger.debug("Enforcing config deltas");
            // unfortunately, previous account info is always null on startup; this will trigger a
            // synchronization on each startup
            enforceConfigurationDeltas(newAccountInfo, previousXmppAccountInfo);
            logger.debug("Done enforcing config deltas");
            logger.debug("Pruning unwanted chatrooms");
            pruneUnwantedXmppChatrooms( newAccountInfo );
            logger.debug("Dome pruning unwanted chatrooms");
            logger.debug("Pruning unwanted chatroom services");
            pruneUnwantedXmppChatRoomServices( newAccountInfo.getXmppChatRooms() );
            logger.debug("Done pruning unwanted chatroom services");
        } else {
            Collection<XmppConfigurationElement> confElements = new ArrayList<XmppConfigurationElement>();
            confElements.addAll(newAccountInfo.getXmppChatRooms());
            enforceConfigurationDeltas(newAccountInfo, previousXmppAccountInfo, confElements);
            logger.debug("Pruning unwanted chatrooms");
            pruneUnwantedXmppChatrooms( newAccountInfo );
            logger.debug("Dome pruning unwanted chatrooms");
            logger.debug("Pruning unwanted chatroom services");
            pruneUnwantedXmppChatRoomServices( newAccountInfo.getXmppChatRooms() );
            logger.debug("Done pruning unwanted chatroom services");
        }
        previousXmppAccountInfo = newAccountInfo;
    }

    private static void enforceConfigurationDeltas(XmppAccountInfo newAccountInfo,
            XmppAccountInfo previousXmppAccountInfo) {
        enforceConfigurationDeltas(newAccountInfo, previousXmppAccountInfo, newAccountInfo.getAllElements());
    }

    private static void enforceConfigurationDeltas(XmppAccountInfo newAccountInfo,
            XmppAccountInfo previousXmppAccountInfo, Collection<XmppConfigurationElement> elements) {
        setElementsChangeStatusBasedOnPreviousConfiguration(newAccountInfo, previousXmppAccountInfo);
        for (XmppConfigurationElement element : elements) {
            if (element.getStatus() != XmppAccountStatus.UNCHANGED) {
                try {
                    element.update();
                } catch (Exception e) {
                    logger.error("setElementsChangeStatusBasedOnPreviousConfiguration caught ", e);
                }
            }
        }
    }

    private static void setElementsChangeStatusBasedOnPreviousConfiguration(XmppAccountInfo newXmppAccountInfo,
            XmppAccountInfo previousXmppAccountInfo) {
        if (previousXmppAccountInfo == null) {
            // no previous config, everything looks new to us.
            for (XmppConfigurationElement element : newXmppAccountInfo.getAllElements()) {
                element.setStatus(XmppAccountStatus.NEW);
            }
        } else {
            // process users
            setElementsChangeStatusBasedOnPreviousConfiguration(newXmppAccountInfo.getXmppUserAccountMap(),
                    previousXmppAccountInfo.getXmppUserAccountMap());
            // process groups
            setElementsChangeStatusBasedOnPreviousConfiguration(newXmppAccountInfo.getXmppGroupMap(),
                    previousXmppAccountInfo.getXmppGroupMap());
            // process chatrooms
            setElementsChangeStatusBasedOnPreviousConfiguration(newXmppAccountInfo.getXmppChatRoomMap(),
                    previousXmppAccountInfo.getXmppChatRoomMap());
        }
        if (logger.isEnabledFor(Level.INFO)) {
            for (XmppConfigurationElement element : newXmppAccountInfo.getAllElements()) {
                logger.info(element.toString());
            }

        }
    }

    private static void setElementsChangeStatusBasedOnPreviousConfiguration(
            Map<String, ? extends XmppConfigurationElement> newXmppAccountMap,
            Map<String, ? extends XmppConfigurationElement> previousXmppAccountMap) {
        for (String elementName : newXmppAccountMap.keySet()) {
            // check if element taken from new account map used to exist in the previous one
            XmppConfigurationElement elementFromNewConfig = newXmppAccountMap.get(elementName);
            XmppConfigurationElement elementFromPreviousConfig = previousXmppAccountMap.get(elementName);
            if (elementFromPreviousConfig == null) {
                // element not found in previous configuration, it must be new
                elementFromNewConfig.setStatus(XmppAccountStatus.NEW);

            } else {
                // the element is not new, check if it has changed
                if (elementFromNewConfig.equals(elementFromPreviousConfig) == true) {
                    elementFromNewConfig.setStatus(XmppAccountStatus.UNCHANGED);
                } else {
                    elementFromNewConfig.setStatus(XmppAccountStatus.MODIFIED);
                }
            }
        }
    }

    private static void pruneUnwantedXmppUsers(Set<String> xmppUserAccountNamesMasterList) {
        SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
        // recall user accounts currently configured in openfire
        Collection<UserAccount> userAccountsInOpenfire = plugin.getUserAccounts();

        // remove all those accounts currently configured in openfire that
        // do not show up on the provided master list
        for (UserAccount userAccountInOpenfire : userAccountsInOpenfire) {
            if (xmppUserAccountNamesMasterList.contains(userAccountInOpenfire.getXmppUserName()) == false) {
                if (!userAccountInOpenfire.getXmppUserName().equals("admin")) {
                    try {
                        // remove user from all the groups it used to belong to - this forces an
                        // immediate update of the groups in IM clients
                        String jidAsString = XmppAccountInfo.appendDomain(userAccountInOpenfire.getXmppUserName());
                        JID jid = new JID(jidAsString);
                        Collection<Group> groups = GroupManager.getInstance().getGroups(jid);
                        for (Group group : groups) {
                            logger.debug("pruneUnwantedXmppUsers removing "
                                    + userAccountInOpenfire.getXmppUserName() + " from group " + group.getName());
                            group.getMembers().remove(jid);
                        }
                        logger.info("Pruning Unwanted Xmpp User " + userAccountInOpenfire.getXmppUserName());
                        plugin.destroyUser(XmppAccountInfo.appendDomain(userAccountInOpenfire.getXmppUserName()));
                        logger.debug("User removed from allowed to create chat rooms users list");
                    } catch (Exception e) {
                        logger.error("pruneUnwantedXmppUsers caught ", e);
                    }
                }
            }
        }
    }

    private static void pruneUnwantedXmppGroups(Set<String> xmppGroupNamesMasterList) {
        SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
        // recall groups currently configured in openfire
        Collection<Group> groupsInOpenfire = plugin.getGroups();

        // remove all those groups currently configured in openfire that
        // do not show up on the provided master list
        for (Group groupInOpenfire : groupsInOpenfire) {
            if (xmppGroupNamesMasterList.contains(groupInOpenfire.getName()) == false) {
                try {
                    plugin.deleteGroup(groupInOpenfire.getName());
                    logger.info("Pruning Unwanted Xmpp group " + groupInOpenfire.getName());
                } catch (Exception e) {
                    logger.error("pruneUnwantedXmppGroups caught ", e);
                }
            }
        }
    }

    private static void pruneUnwantedXmppChatrooms(XmppAccountInfo newAccountInfo) {
        SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
        // recall chatrooms currently configured in openfire
        Collection<MUCRoom> chatRoomsInOpenfire = plugin.getMUCRooms();

        // remove all those groups currently configured in openfire that
        // do not show up on the provided master list
        for (MUCRoom mucRoomInOpenfire : chatRoomsInOpenfire) {
            String domain = mucRoomInOpenfire.getMUCService().getServiceDomain().split("\\.")[0];
            if (newAccountInfo.getXmppChatRoom(domain, mucRoomInOpenfire.getName()) == null) {
                // openfire chatroom not found in the account info - remove it if not ad hoc
                if (!mucRoomInOpenfire.wasSavedToDB()) {
                    /*
                     * Anything not in our database is deemed adhoc - enforce logging so that even
                     * ad hoc rooms get logged.
                     */
                    logger.debug("Adhoc chat room detected - enable logging");
                    mucRoomInOpenfire.setLogEnabled(true);
                } else {
                    logger.info("Pruning Unwanted Xmpp chatroom " + domain + ":" + mucRoomInOpenfire.getName());
                    mucRoomInOpenfire.destroyRoom(null, "not a managed chat");
                    ProviderFactory.getMUCProvider().deleteFromDB(mucRoomInOpenfire);
                    // when IM room is deleted, delete bookmark as well if necessary
                    if (SipXBookmarkManager.isInitialized()) {
                        SipXBookmarkManager manager = SipXBookmarkManager.getInstance();
                        if (manager.getMUCBookmarkID(mucRoomInOpenfire.getName()) != null)  {
                            manager.deleteMUCBookmark(mucRoomInOpenfire.getName());
                        }
                    }
                }
            }
        }
    }

    private static void pruneUnwantedXmppChatRoomServices(Collection<XmppChatRoom> configuredXmppChatRooms) {
        /*
         * Restrict the chat services to those contained configured chatrooms
         */
        HashSet<String> allowedDomains = new HashSet<String>();
        for (XmppChatRoom configuredChatRoom : configuredXmppChatRooms) {
            allowedDomains.add(configuredChatRoom.getSubdomain());
        }
        SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
        try {
            plugin.pruneChatServices(allowedDomains);
        } catch (Exception e) {
            logger.error("pruneUnwantedXmppChatRoomServices caught ", e);
        }
    }

    public AccountsParser(String watchFileName, ConferenceService conferenceService, boolean parsingEnabled) {
        this.watchFile = new File(watchFileName);
        m_parsingEnabled = parsingEnabled;
        m_conferenceService = conferenceService;
    }

    public void startScanner() {
        Scanner scanner = new Scanner();
        timer.schedule(scanner, 0, 10000);
    }

    public static void stopScanner() {
        timer.cancel();
    }

    public static XmppAccountInfo parse(ConferenceService conferenceService, boolean parseEnabled) {
        try {
            XmppAccountInfo accountInfo = new XmppAccountInfo();
            if (parseEnabled) {
                parseMongoUsers(accountInfo);
                parseMongoGroups(accountInfo);
            }
            parseMongoConferences(accountInfo, conferenceService);

            return accountInfo;
        } catch (Exception ex) {
            logger.error(ex);
            throw new SipXOpenfirePluginException(ex);
        }

    }

    private static void parseMongoUsers(XmppAccountInfo accountInfo) {
        ValidUsers validUsers = UnfortunateLackOfSpringSupportFactory.getValidUsers();
        List<User> users = validUsers.getUsersWithImEnabled();
        XmppUserAccount account = null;
        for (User user : users) {
            account = new XmppUserAccount();
            account.setPassword(user.getPintoken());
            account.setUserName(user.getJid());
            account.setSipUserName(user.getUserName());
            account.setEmail(StringUtils.defaultIfEmpty(user.getEmailAddress(), StringUtils.EMPTY));
            account.setDisplayName(StringUtils.defaultIfEmpty(user.getImDisplayName(), StringUtils.EMPTY));
            account.setAdvertiseOnCallPreference(BooleanUtils.toStringTrueFalse(user.isAdvertiseOnCallStatus()));
            account.setOnThePhoneMessage(StringUtils.defaultIfEmpty(user.getOnthePhoneMessage(), StringUtils.EMPTY));

            accountInfo.addAccount(account);
        }
        User imbotUser = validUsers.getImbotUser();
        account = new XmppUserAccount();
        account.setPassword(imbotUser.getPintoken());
        account.setUserName(imbotUser.getUserName());
        accountInfo.addAccount(account);
    }

    private static void parseMongoConferences(XmppAccountInfo accountInfo, ConferenceService conferenceService) throws Exception {
        List<Conference> conferences = conferenceService.getAllConferences();
        ValidUsers users = UnfortunateLackOfSpringSupportFactory.getValidUsers();
        User user;
        for (Conference conference : conferences) {
            user = users.getUser(conference.getConfOwner());
            if (user == null || !user.isImEnabled()) {
                continue;
            }
            XmppChatRoom chatRoom = new XmppChatRoom();
            chatRoom.setConferenceName(conference.getConfName());
            chatRoom.setRoomName(conference.getConfName());
            chatRoom.setSubdomain("conference");
            chatRoom.setOwner(user.getJid());
            chatRoom.setDescription(conference.getConfDescription());
            chatRoom.setPassword(conference.getPin());
            chatRoom.setModerated(new Boolean(conference.isModerated()).toString());
            chatRoom.setIsPublicRoom(new Boolean(conference.isPublic()).toString());
            chatRoom.setMembersOnly(new Boolean(conference.isMembersOnly()).toString());
            chatRoom.setPersistent(Boolean.TRUE.toString());
            chatRoom.setConferenceExtension(conference.getExtension());

            accountInfo.addChatRoom(chatRoom);
        }
    }

    private static void parseMongoGroups(XmppAccountInfo accountInfo) throws Exception {
        ValidUsers validUsers = UnfortunateLackOfSpringSupportFactory.getValidUsers();
        Collection<UserGroup> groups = validUsers.getImGroups();
        XmppGroupMember member = null;
        for (UserGroup group : groups) {
            XmppGroup xmppGroup = new XmppGroup();
            String groupName = group.getGroupName();
            xmppGroup.setGroupName(groupName);
            xmppGroup.setDescription(StringUtils.defaultIfEmpty(group.getDescription(), StringUtils.EMPTY));
            List<String> imIds = validUsers.getAllImIdsInGroup(groupName);
            for (String imId : imIds) {
                member = new XmppGroupMember();
                member.setUserName(imId);
                xmppGroup.addMember(member);
            }
            if (group.isImbotEnabled()) {
                member = new XmppGroupMember();
                member.setUserName(validUsers.getImBotName());
                xmppGroup.addMember(member);
            }

            accountInfo.addGroup(xmppGroup);
        }
    }
}
