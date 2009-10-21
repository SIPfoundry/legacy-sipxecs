package org.sipfoundry.openfire.plugin.presence;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.jivesoftware.openfire.PacketRouter;
import org.jivesoftware.openfire.PresenceManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.container.PluginManager;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupAlreadyExistsException;
import org.jivesoftware.openfire.group.GroupManager;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.jivesoftware.openfire.interceptor.InterceptorManager;
import org.jivesoftware.openfire.muc.MUCRole;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.muc.MultiUserChatManager;
import org.jivesoftware.openfire.muc.MultiUserChatService;
import org.jivesoftware.openfire.muc.NotAllowedException;
import org.jivesoftware.openfire.muc.MUCRole.Affiliation;
import org.jivesoftware.openfire.spi.PresenceManagerImpl;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserAlreadyExistsException;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.jivesoftware.util.NotFoundException;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.openfire.config.AccountsParser;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.sipcallwatcher.CallWatcher;
import org.sipfoundry.sipcallwatcher.ResourceStateChangeListener;
import org.xmpp.component.Component;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;
import org.xmpp.packet.JID;
import org.xmpp.packet.Packet;
import org.xmpp.packet.Presence;

public class SipXOpenfirePlugin implements Plugin, Component {

    private MultiUserChatManager multiUserChatManager;
    private GroupManager groupManager;
    private UserManager userManager;
    private PresenceManager presenceManager;
    private PluginManager pluginManager;
    private ComponentManager componentManager;
    private String hostname;
    private Map<String, Presence> probedPresence;
    private JID componentJID;
    private XMPPServer server;

    private static String configurationPath = "/etc/sipxpbx";

    private static WatcherConfig watcherConfig;

    private static String logFile;

    private static SipFoundryAppender logAppender;

    public static final String SIP_UID = "sipUid";

    public static final String SIP_PWD = "sipPwd";

    public static final String ON_THE_PHONE_MESSAGE = "onThePhoneMessage";

    public static final String CONFERENCE_EXTENSION = "conferenceExtension";

    private static Logger log = Logger.getLogger(SipXOpenfirePlugin.class);

    private static SipXOpenfirePlugin instance;

    private Map<String, String> sipIdToXmppIdMap = new HashMap<String, String>();

    private Map<String, ConferenceInformation> roomNameToConferenceInfoMap = new HashMap<String, ConferenceInformation>();

    private AccountsParser accountsParser;

    public class ConferenceInformation {
        private String extension;
        private String name;
        private String pin;
        private String reachabilityInfo;

        public String getExtension() {
            return extension;
        }

        public void setExtension(String extension) {
            this.extension = extension;
        }

        public String getPin() {
            return pin;
        }

        public void setPin(String pin) {
            this.pin = pin;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public ConferenceInformation(String name, String extension, String pin, String reachabilityInfo) {
            this.name = name;
            this.extension = extension;
            this.pin = pin;
            this.reachabilityInfo = reachabilityInfo;
        }

        public String getReachabilityInfo() {
            return reachabilityInfo;
        }

        public void setReachabilityInfo( String reachabilityInfo ) {
            this.reachabilityInfo = reachabilityInfo;
        }
    }

    static void parseConfigurationFile() {
        String configurationFile = configurationPath + "/sipxopenfire.xml";

        if (!new File(configurationFile).exists()) {
            String msg = String.format("Configuration %s file not found -- exiting",
                    configurationFile);
            System.err.println(msg);
            throw new SipXOpenfirePluginException(msg);
        }
        ConfigurationParser parser = new ConfigurationParser();
        watcherConfig = parser.parse("file://" + configurationFile);
        logFile = watcherConfig.getLogDirectory() + "/sipxopenfire.log";

    }

    static void initializeLogging() throws SipXOpenfirePluginException {
        try {
            String javaClassPaths = System.getProperty("java.class.path");
            String openfireHome = System.getProperty("openfire.home");
            StringBuilder sb = new StringBuilder(javaClassPaths).append(":" + openfireHome
                    + "/lib/sipxcommons.jar");
            System.setProperty("java.class.path", sb.toString());
            String log4jPropertiesFile = configurationPath + "/log4j.properties";

            if (new File(log4jPropertiesFile).exists()) {
                /*
                 * Override the file configuration setting.
                 */
                Properties props = new Properties();
                props.load(new FileInputStream(log4jPropertiesFile));
                String level = props.getProperty("log4j.category.org.sipfoundry.openfire");
                if (level != null) {
                    watcherConfig.setLogLevel(level);
                }
            }
            setLogAppender(new SipFoundryAppender(new SipFoundryLayout(), logFile));
            // TODO -- this should be org.sipfoundry.openfire.
            Logger applicationLogger = Logger.getLogger("org.sipfoundry");

            /*
             * Set the log level.
             */
            if (watcherConfig.getLogLevel().equals("TRACE")) {
                applicationLogger.setLevel(org.apache.log4j.Level.DEBUG);
            } else {
                applicationLogger.setLevel(org.apache.log4j.Level.toLevel(watcherConfig
                        .getLogLevel()));
            }

            applicationLogger.addAppender(getLogAppender());
            if (System.getProperty("output.console") != null) {
                applicationLogger.addAppender(new ConsoleAppender(new PatternLayout()));
            }

            
        } catch (Exception ex) {
            throw new SipXOpenfirePluginException(ex);
        }
    }

    public void initializePlugin(PluginManager manager, File pluginDirectory) {
        SipXOpenfirePlugin.instance = this;

        InputStream in = getClass().getResourceAsStream("/config.properties");
        Properties properties = new Properties();
        componentManager = ComponentManagerFactory.getComponentManager();

        try {
            properties.load(in);
        } catch (IOException ex) {
            componentManager.getLog().error(ex);
        }

        try {
            if (new File("/tmp/sipx.properties").exists()) {
                System.getProperties()
                        .load(new FileInputStream(new File("/tmp/sipx.properties")));
            }
        } catch (Exception ex) {
            componentManager.getLog().error(ex);
            throw new SipXOpenfirePluginException("Error reading config file ", ex);
        }

        pluginManager = manager;
        ClassLoader classLoader = pluginManager.getPluginClassloader(this);
        classLoader.setPackageAssertionStatus("org.sipfoundry", true);
        Thread.currentThread().setContextClassLoader(classLoader);
        configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
        parseConfigurationFile();
        initializeLogging();
        server = XMPPServer.getInstance();

        userManager = server.getUserManager();
        presenceManager = server.getPresenceManager();

        hostname = server.getServerInfo().getXMPPDomain();
        log.info("HostName = " + hostname);

        probedPresence = new ConcurrentHashMap<String, Presence>();

        groupManager = GroupManager.getInstance();

        multiUserChatManager = server.getMultiUserChatManager();

        /*
         * Load up the database.
         */
        log.info("hostname " + hostname);

        String accountConfigurationFile = configurationPath + "/xmpp-account-info.xml";
        if (!new File(accountConfigurationFile).exists()) {
            System.err.println("User account file not found");
            throw new SipXOpenfirePluginException("Cannot find user accounts file");
        } else {
            this.accountsParser = new AccountsParser(accountConfigurationFile);
            this.accountsParser.startScanner();
            try{
                Thread.sleep(10000);  //leave enough time to allow configuration enforcement
            }
            catch( Exception ex ){
            }
        }

        // config and instantiate and the presence unifier used to gather all presence info
        PresenceUnifier.setPlugin(this);
        PresenceUnifier.getInstance();
        // add packet interceptor

        InterceptorManager.getInstance().addInterceptor(new MessagePacketInterceptor(this));
        log.info("plugin initializaton completed");
        log.info("DONE");

        CallWatcher.setWatcherConfig(watcherConfig);

        /*
         * Everything else is ready.  Let the SIP side of the show begin...
         */
        try {
            CallWatcher.pluginInit();
            log.info("completed init");
            ResourceStateChangeListener resourceStateChangeListener = new ResourceStateChangeListenerImpl(
                    this);
            CallWatcher.getSubscriber().setResourceStateChangeListener(
                    resourceStateChangeListener);
        } catch (Exception e) {
            log.error("Error initializing CallWatcher", e);
            throw new SipXOpenfirePluginException("Init error", e);
        }
    }

    public void destroyPlugin() {
        log.debug("DestroyPlugin");
    }

    public String getName() {
        return pluginManager.getName(this);
    }

    public String getDescription() {
        return pluginManager.getDescription(this);
    }

    public void initialize(JID jid, ComponentManager componentManager) {
    }

    public void start() {
    }

    public void shutdown() {
    }

    public void processPacket(Packet packet) {
        // Check that we are getting an answer to a presence probe
        if (packet instanceof Presence) {
            Presence presence = (Presence) packet;
            if (presence.isAvailable() || presence.getType() == Presence.Type.unavailable
                    || presence.getType() == Presence.Type.error) {
                // Store answer of presence probes
                probedPresence.put(presence.getFrom().toString(), presence);
            }
        }
    }

    // Experimental - to be debugged and tested
    public Presence getForeignUserPresence(String sender, String jid)
            throws UserNotFoundException {

        log.debug("getPresence : sender = " + sender + " jid = " + jid);

        if (jid == null) {
            throw new UserNotFoundException("Target JID not found in request");
        }

        JID targetJID = new JID(jid);
        // Check that the sender is not requesting information of a remote server entity
        if (targetJID.getDomain() == null || XMPPServer.getInstance().isRemote(targetJID)) {
            throw new UserNotFoundException("Domain does not matches local server domain");
        }
        if (!hostname.equals(targetJID.getDomain())) {
            // Sender is requesting information about component presence, so we send a
            // presence probe to the component.
            presenceManager.probePresence(componentJID, targetJID);

            // Wait 30 seconds until we get the probe presence result
            int count = 0;
            Presence presence = probedPresence.get(jid);
            while (presence == null) {
                if (count > 300) {
                    // After 30 seconds, timeout
                    throw new UserNotFoundException(
                            "Request for component presence has timed-out.");
                }
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    // don't care!
                }
                presence = probedPresence.get(jid);

                count++;
            }
            // Clean-up probe presence result
            probedPresence.remove(jid);
            // Return component presence
            return presence;
        }
        if (targetJID.getNode() == null
                || !UserManager.getInstance().isRegisteredUser(targetJID.getNode())) {
            // Sender is requesting presence information of an anonymous user
            throw new UserNotFoundException("Username is null");
        }
        User user = userManager.getUser(targetJID.getNode());
        log.debug("user = " + user.getName());
        log.debug("isNameVisible " + user.isNameVisible());
        return ((PresenceManagerImpl) presenceManager).getPresence(user);

    }

    public void setPresenceState(String jid, UnifiedPresence.XmppPresence xmppPresence)
            throws UserNotFoundException {
        log.debug("setPresenceState : " + jid + " XmppPresence = " + xmppPresence);
        if (jid == null) {
            throw new UserNotFoundException("Target JID not found in request");
        }
        JID targetJID = new JID(jid);
        // Check that the sender is not requesting information of a remote server entity
        if (targetJID.getDomain() == null || XMPPServer.getInstance().isRemote(targetJID)) {
            throw new UserNotFoundException("Domain does not matches local server domain");
        }
        User user = userManager.getUser(targetJID.getNode());
        Presence presence = presenceManager.getPresence(user);
        if (presence == null) {
            log.debug("User is OFFLINE  -- cannot set presence state");
            return;
        }

        presence.setShow(xmppPresence.asPresenceShowEnum());
        user.getRoster().broadcastPresence(presence);

    }

    public void setPresenceStatus(String jid, String presenceMessage)
            throws UserNotFoundException {

        log.debug("setPresenceStatus jid = " + jid + " presenceMessage = " + presenceMessage);
        if (jid == null) {
            throw new UserNotFoundException("Target JID not found in request");
        }
        JID targetJID = new JID(jid);
        // Check that the sender is not requesting information of a remote server entity
        if (targetJID.getDomain() == null || XMPPServer.getInstance().isRemote(targetJID)) {
            throw new UserNotFoundException("Domain does not matches local server domain");
        }
        User user = userManager.getUser(targetJID.getNode());

        Presence presence = presenceManager.getPresence(user);

        if (presence == null) {
            log.debug("User is OFFLINE  -- cannot set presence state");
            return;
        }
        presence.setStatus(presenceMessage);

        user.getRoster().broadcastPresence(presence);
    }

    public boolean userHasAccount(String jid) {
        log.debug("userHasAccount : " + jid);
        try {
            if (jid == null) {
                throw new NullPointerException("Target JID not found in request");
            }
            JID targetJID = new JID(jid);
            // Check that the sender is not requesting information of a remote server entity
            if (targetJID.getDomain() == null || XMPPServer.getInstance().isRemote(targetJID)) {
                return false;
            }
            User user = userManager.getUser(targetJID.getNode());
            if (user == null) {
                return false;
            }
            return true;
        } catch (UserNotFoundException ex) {
            return false;
        }
    }

    public void createUserAccount(String userName, String password, String displayName,
            String email) {

        try {
            User user = userManager.getUser(userName);
            user.setPassword(password);
            user.setName(displayName);
            user.setEmail(email);
        } catch (UserNotFoundException e) {
            try {
                userManager.createUser(userName, password, displayName, email);
            } catch (UserAlreadyExistsException ex) {
                throw new SipXOpenfirePluginException(ex);
            }
        } catch (Exception ex) {
            throw new SipXOpenfirePluginException(ex);
        }
        
    }

    public void destroyUser(String jid) throws UserNotFoundException {
        try {
            log.info("destroyUser " + jid);

            JID targetJID = new JID(jid);

            User user = userManager.getUser(targetJID.getNode());
            if (user == null) {
                log.info("User not found " + jid);
                return;
            }
            userManager.deleteUser(user);
            // Remove user from all groups where he is a member.
            groupManager.deleteUser(user);
        } catch (UserNotFoundException ex) {
            log.debug("Could not find user " + jid);
        }
    }

    public String getSipId(String userName) throws UserNotFoundException {
        User user = userManager.getUser(userName);
        return user.getProperties().get(SIP_UID);
    }

    // associates a jid with a sip username (without the SIP domain part) 
    public void setSipId(String userName, String sipUserName) throws UserNotFoundException {
        User user = userManager.getUser(userName);
        user.getProperties().put(SIP_UID, sipUserName);
        user.getProperties().put(ON_THE_PHONE_MESSAGE, "I am on the phone");
        this.sipIdToXmppIdMap.put(sipUserName, userName);
    }

    public String getSipPassword(String userName) throws UserNotFoundException {
        User user = userManager.getUser(userName);
        return user.getProperties().get(SIP_PWD);
    }

    public void setSipPassword(String userName, String sipPassword) throws UserNotFoundException {
        User user = userManager.getUser(userName);
        user.getProperties().put(SIP_PWD, sipPassword);
    }

    public void createGroup(String groupName, JID adminJID, String description)
            throws IllegalArgumentException {
        log.debug("createGroup groupName = " + groupName);
        Group group = null;
        
        try {
            group = groupManager.getGroup(groupName);
            groupManager.deleteGroup(group);

        } catch (GroupNotFoundException ex) {

        }
        try {
            group = groupManager.createGroup(groupName);

        } catch (GroupAlreadyExistsException ex1) {
            log.debug("Group already exists - not creating group");

        }
        if (description != null) {
            group.setDescription(description);
        }

        if (adminJID != null && adminJID.getDomain().equals(this.getXmppDomain())
                && this.isValidUser(adminJID)) {
            group.getAdmins().add(adminJID);
        }
        
        // configure group as 'shared' among group members
        // (magic recipe taken from 'AddGroup.java file of openfire project)
        group.getProperties().put("sharedRoster.showInRoster", "onlyGroup");
        group.getProperties().put("sharedRoster.displayName", groupName);
    }

    public void addUserToGroup(JID jid, String groupName, boolean isAdmin)
            throws GroupNotFoundException {
        log.debug("addUserToGroup " + jid + " GroupName " + groupName);

        Group group = groupManager.getGroup(groupName, true);
        if (group.getAdmins().contains(jid)) {
            log.debug("Admins already has " + jid);
            group.getMembers().add(jid);
        } else {
            if (isAdmin) {
                if (jid.getDomain().equals(this.getXmppDomain())) {
                    group.getAdmins().add(jid);
                }
            }
            group.getMembers().add(jid);
        }

    }

    public void removeUserFromGroup(String userJid, String groupName)
            throws GroupNotFoundException {
        JID jid = new JID(userJid);
        Group group = groupManager.getGroup(groupName, true);
        group.getMembers().remove(jid);
        group.getAdmins().remove(jid);
    }

    public void deleteGroup(String groupName) throws GroupNotFoundException {
        Group group = groupManager.getGroup(groupName);
        groupManager.deleteGroup(group);

    }

    public void setOnThePhoneMessage(String sipUserName, String onThePhoneMessage)
            throws UserNotFoundException {
        String userName = this.sipIdToXmppIdMap.get(sipUserName);
        if (userName == null) {
            throw new UserNotFoundException("SIP User " + sipUserName + " not found");
        }
        User user = userManager.getUser(userName);
        if (!onThePhoneMessage.equals("")) {
            user.getProperties().put(ON_THE_PHONE_MESSAGE, onThePhoneMessage);
        } else {
            user.getProperties().remove(ON_THE_PHONE_MESSAGE);
        }
    }

    public String getOnThePhoneMessage(String jid) throws UserNotFoundException {

        User user = userManager.getUser(jid);
        return user.getProperties().get(ON_THE_PHONE_MESSAGE);

    }

    public void setConferenceExtension(String sipUserName, String conferenceExtension)
            throws UserNotFoundException {
        String userName = this.sipIdToXmppIdMap.get(sipUserName);
        if (userName == null) {
            throw new UserNotFoundException("SIP User " + sipUserName + " not found");
        }
        User user = userManager.getUser(userName);

        user.getProperties().put(CONFERENCE_EXTENSION, conferenceExtension);

    }

    public boolean groupExists(String groupName) {
        try {
            Group group = groupManager.getGroup(groupName);
            return true;
        } catch (GroupNotFoundException ex) {
            return false;
        }
    }

    public String getPresenceStatus(String jid) throws UserNotFoundException {
        log.debug("getPresenceStatus " + jid);
        JID targetJID = new JID(jid);
        User user = userManager.getUser(targetJID.getNode());
        Presence presence = presenceManager.getPresence(user);
        if (presence == null) {
            log.debug("User is offline");
            return "";
        }
        return presence.getStatus();
    }

    public void stopServer() throws Exception {
        server.stop();
    }

    public void startServer() throws Exception {
        server.start();
    }

    // returns the JID given a sip user part.
    public String getXmppId(String sipId) {
        return this.sipIdToXmppIdMap.get(sipId);
    }

    /**
     * @return the server
     */
    public XMPPServer getServer() {
        return server;
    }

    public String getXmppDomain() {
        return this.server.getServerInfo().getXMPPDomain();
    }

    public boolean isUserInGroup(String jid, String groupName) throws GroupNotFoundException {
        Group group = groupManager.getGroup(groupName, true);
        return group.isUser(jid);
    }

    public void destroyMultiUserChatService(String subdomain) {
        try {
            this.multiUserChatManager.removeMultiUserChatService(subdomain);

        } catch (NotFoundException nfe) {
            log.debug("destroyMultiUserChatService not found " + subdomain);
        }
    }
    
    
    public void createChatRoom(String subdomain, String ownerJid, String roomName,
            boolean makeRoomModerated, boolean makeRoomMembersOnly,
            boolean allowOccupantsToInviteOthers, boolean isPublicRoom,
            boolean logRoomConversations, boolean isPersistent, String password,
            String description, String conferenceExtension, String conferenceName, String conferenceReachabilityInfo)
            throws Exception {
        MultiUserChatService mucService = XMPPServer.getInstance().getMultiUserChatManager()
                .getMultiUserChatService(subdomain);
        if (mucService == null) {
            mucService = XMPPServer.getInstance().getMultiUserChatManager()
                    .createMultiUserChatService(subdomain, description, false);
            Collection<JID> admins = XMPPServer.getInstance().getAdmins();
            JID admin = admins.iterator().next();
            mucService.addUserAllowedToCreate(admin.toBareJID());
            mucService.addUserAllowedToCreate(ownerJid);
            mucService.addSysadmin(admin.toBareJID());
            mucService.setLogConversationsTimeout(60);
            mucService.setLogConversationBatchSize(100);
            mucService.setRoomCreationRestricted(true);
        }

        MUCRoom mucRoom = mucService.getChatRoom(roomName, new JID(ownerJid));

        mucRoom.unlock(mucRoom.getRole());

        if (!mucRoom.wasSavedToDB()) {
            mucRoom.saveToDB();
        }

        if (mucRoom.isPersistent() != isPersistent) {
            mucRoom.setPersistent(isPersistent);
        }

        if (!mucRoom.getOwners().contains(ownerJid)) {
            mucRoom.addOwner(ownerJid, mucRoom.getRole());

        }

        for (JID admins : XMPPServer.getInstance().getAdmins()) {
            if (!mucRoom.getOwners().contains(admins.toBareJID())) {
                mucRoom.addOwner(ownerJid, mucRoom.getRole());
            }
        }

        if (!mucRoom.canAnyoneDiscoverJID()) {
            mucRoom.setCanAnyoneDiscoverJID(true);
        }

        if (!mucRoom.canChangeNickname()) {
            mucRoom.setChangeNickname(true);
        }

        if (mucRoom.isModerated() != makeRoomModerated) {
            mucRoom.setModerated(makeRoomModerated);
        }
        if (mucRoom.isMembersOnly() != makeRoomMembersOnly) {
            mucRoom.setMembersOnly(makeRoomMembersOnly);
        }

        if (!mucRoom.isRegistrationEnabled()) {
            mucRoom.setRegistrationEnabled(true);
        }

        if (mucRoom.isPublicRoom() != isPublicRoom) {
            mucRoom.setPublicRoom(isPublicRoom);
        }

        if (mucRoom.canOccupantsInvite() != allowOccupantsToInviteOthers) {
            mucRoom.setCanOccupantsInvite(allowOccupantsToInviteOthers);

        }

        if (mucRoom.getDescription() != null && description == null
                || mucRoom.getDescription() == null && description != null
                || mucRoom.getDescription() != description
                || !mucRoom.getDescription().equals(description)) {
            mucRoom.setDescription(description);

        }

        /*
         * Check if password changed and set password if changed.
         */

        if (mucRoom.getPassword() != null && password == null || mucRoom.getPassword() == null
                && password != null || mucRoom.getPassword() != password
                || !mucRoom.getPassword().equals(password)) {
            mucRoom.setPassword(password);

        }

        if (!mucRoom.canOccupantsChangeSubject()) {

            mucRoom.setCanOccupantsChangeSubject(true);
        }

        if (!mucRoom.canChangeNickname()) {
            mucRoom.setChangeNickname(true);
        }

        if (mucRoom.isLogEnabled() != logRoomConversations) {
            mucRoom.setLogEnabled(logRoomConversations);
        }

        /* The conference extension is the voice conf bridge extension */
        this.roomNameToConferenceInfoMap.put(subdomain + "." + roomName,
                new ConferenceInformation(conferenceName, conferenceExtension, password, conferenceReachabilityInfo));

    }

    /**
     * Delete a chat room from the domain.
     * 
     * @param domain
     * @param roomName
     */
    public void removeChatRoom(String domain, String roomName) {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        log.debug("removeChatRoom domain = " + domain + " roomName = " + roomName);
        mucService.removeChatRoom(roomName);
        this.roomNameToConferenceInfoMap.remove(domain + "." + roomName);
    }

    /**
     * Returns a chat room by name
     * 
     * @param domain
     * @param roomName
     * @return reference to multi=user chat room if found, otherwise returns null
     */
    public MUCRoom getChatRoom(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        if (mucService == null) {
            throw new NotFoundException("Service not found for domain " + domain);
        }
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        return mucRoom;
    }

    /**
     * Get all the members of a chat room.
     * 
     * @param domain
     * @param roomName
     * @return
     */
    public Collection<String> getMembers(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        if (mucService == null) {
            throw new NotFoundException("Service not found for domain " + domain);
        }
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return mucRoom.getMembers();
    }

    public Map<String, String> getMucRoomAttributes(String domain, String roomName)
            throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        Map<String, String> retval = new HashMap<String, String>();
        retval.put("isModerated", "" + mucRoom.isModerated());
        retval.put("isLogEnabled", "" + mucRoom.isLogEnabled());
        retval.put("isMembersOnly", "" + mucRoom.isMembersOnly());
        retval.put("isPublicRoom", "" + mucRoom.isPublicRoom());
        retval.put("isLoginRestrictedToNickName", "" + mucRoom.isLoginRestrictedToNickname());
        retval.put("isLocked", "" + mucRoom.isLocked());
        retval.put("isRegistrationEnabled", "" + mucRoom.isRegistrationEnabled());
        retval.put("isPasswordProtected", "" + mucRoom.isPasswordProtected());
        retval.put("canAnyoneDiscoverJID", "" + mucRoom.canAnyoneDiscoverJID());
        retval.put("canChangeNickName", "" + mucRoom.canChangeNickname());
        retval.put("canOccupantsInvite", "" + mucRoom.canOccupantsInvite());
        retval.put("canOccupantsChangeSubject", "" + mucRoom.canOccupantsChangeSubject());
        return retval;
    }

    public void setMucRoomAttributes(String domain, String roomName, Map newAttributes)
            throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        Map<String, String> attribs = (Map<String, String>) newAttributes;
        boolean isModerated = Boolean.parseBoolean(attribs.get("isModerated"));
        mucRoom.setModerated(isModerated);
        boolean isLogEnabled = Boolean.parseBoolean(attribs.get("isLogEnabled"));
        mucRoom.setLogEnabled(isLogEnabled);
        boolean isMembersOnly = Boolean.parseBoolean(attribs.get("isMembersOnly"));
        mucRoom.setMembersOnly(isMembersOnly);
        boolean isPublicRoom = Boolean.parseBoolean(attribs.get("isPublicRoom"));
        mucRoom.setPublicRoom(isPublicRoom);
        boolean isLoginRestrictedToNickName = Boolean.parseBoolean(attribs
                .get("isLoginRestrictedToNickName"));
        mucRoom.setLoginRestrictedToNickname(isLoginRestrictedToNickName);
        boolean isRegistrationEnabled = Boolean
                .parseBoolean(attribs.get("isRegistrationEnabled"));
        mucRoom.setRegistrationEnabled(isRegistrationEnabled);
        boolean canAnyoneDiscoverJID = Boolean.parseBoolean(attribs.get("canAnyoneDiscoverJID"));
        mucRoom.setCanAnyoneDiscoverJID(canAnyoneDiscoverJID);
        boolean canChangeNickName = Boolean.parseBoolean(attribs.get("canChangeNickName"));
        mucRoom.setChangeNickname(canChangeNickName);
        boolean canOccupantsInvite = Boolean.parseBoolean(attribs.get("canOccupantsInvite"));
        mucRoom.setCanOccupantsInvite(canOccupantsInvite);
        boolean canOccupantsChangeSubject = Boolean.parseBoolean(attribs
                .get("canOccupantsChangeSubject"));
        mucRoom.setCanOccupantsChangeSubject(canOccupantsChangeSubject);
    }

    public String getConferenceName(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        ConferenceInformation confInfo = this.roomNameToConferenceInfoMap.get(domain + "."
                + roomName);
        ;
        if (confInfo == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return confInfo.name;
    }

    public String getConferenceExtension(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        ConferenceInformation confInfo = this.roomNameToConferenceInfoMap.get(domain + "."
                + roomName);
        ;
        if (confInfo == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return confInfo.extension;
    }

    public String getConferencePin(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager
                .getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        ConferenceInformation confInfo = this.roomNameToConferenceInfoMap.get(domain + "."
                + roomName);
        ;
        if (confInfo == null) {
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return confInfo.pin;
    }

    public HashSet<UserAccount> getUserAccounts() {
        HashSet<UserAccount> userAccounts = new HashSet<UserAccount>();
        for (User user : this.userManager.getUsers()) {
            UserAccount userAccount = new UserAccount();
            String sipUserId = user.getProperties().get(SIP_UID);
            userAccount.setXmppUserName(user.getUsername());
            userAccount.setSipUserName(sipUserId);
            userAccounts.add(userAccount);
        }
        return userAccounts;
    }

    public static SipXOpenfirePlugin getInstance() {
        return SipXOpenfirePlugin.instance;
    }

    public Collection<Group> getGroups() {
        return this.groupManager.getGroups();
    }

    public Collection<MUCRoom> getMUCRooms() {
        HashSet<MUCRoom> retval = new HashSet<MUCRoom>();
        for (MultiUserChatService mucService : this.multiUserChatManager
                .getMultiUserChatServices()) {
            List<MUCRoom> chatRooms = mucService.getChatRooms();
            retval.addAll(chatRooms);
        }
        return retval;
    }

    public void pruneChatServices(Collection<String> subdomains) throws Exception {

        HashSet<MultiUserChatService> pruneSet = new HashSet<MultiUserChatService>();
        pruneSet.addAll(this.multiUserChatManager.getMultiUserChatServices());

        for (MultiUserChatService service : pruneSet) {
            String subdomain = service.getServiceDomain().split("\\.")[0];
            if (!subdomains.contains(subdomain)) {
                this.multiUserChatManager.removeMultiUserChatService(subdomain);
            }
        }
    }

    public void kickOccupant(String subdomain, String roomName, String password,
            String memberJid, String reason) throws NotAllowedException {
        MUCRoom mucRoom = this.multiUserChatManager.getMultiUserChatService(subdomain)
                .getChatRoom(roomName);
        String roomPassword = mucRoom.getPassword();
        if (password == null && roomPassword != null) {
            throw new NotAllowedException("Password mismatch");
        }
        if (mucRoom.getPassword() != null && !mucRoom.getPassword().equals(password)) {
            throw new NotAllowedException("Password mismatch");
        }
        String actorJid = mucRoom.getOwners().iterator().next();
        JID memberJID = new JID(memberJid);
        JID actorJID = new JID(actorJid);
        mucRoom.kickOccupant(memberJID, actorJID, reason);

    }

    public void inviteOccupant(String subdomain, String roomName, String memberJid,
            String password, String reason) throws Exception {
        MultiUserChatService service = this.multiUserChatManager
                .getMultiUserChatService(subdomain);
        if (service == null) {
            throw new NotFoundException("MUC service not found for " + subdomain);
        }
        MUCRoom mucRoom = service.getChatRoom(roomName);
        if (mucRoom == null) {
            throw new NotFoundException("Room not found for " + subdomain + " roomName "
                    + roomName);
        }
        String roomPassword = mucRoom.getPassword();
        if (password == null && roomPassword != null) {
            throw new NotAllowedException("Password mismatch");
        }
        if (mucRoom.getPassword() != null && !mucRoom.getPassword().equals(password)) {
            throw new NotAllowedException("Password mismatch");
        }
        String ownerJid = mucRoom.getOwners().iterator().next();
        if (!mucRoom.getOccupants().contains(new JID(ownerJid))) {
            throw new NotAllowedException("Owner is not in te room -- cannot invite");
        }

        List<MUCRole> roles = mucRoom.getOccupantsByBareJID(ownerJid);
        JID memberJID = new JID(memberJid);
        for (MUCRole role : roles) {
            if (role.getAffiliation() == Affiliation.owner) {
                mucRoom.sendInvitation(memberJID, reason, role, null);
                break;
            }
        }

    }

    public PacketRouter getPacketRouter() {
        return this.getServer().getPacketRouter();
    }

    public WatcherConfig getSipXopenfireConfig() {
        return watcherConfig;
    }

    /**
     * @param logAppender the logAppender to set
     */
    public static void setLogAppender(SipFoundryAppender logAppender) {
        SipXOpenfirePlugin.logAppender = logAppender;
    }

    /**
     * @return the logAppender
     */
    public static SipFoundryAppender getLogAppender() {
        return logAppender;
    }

    public boolean isValidUser(JID userJid) {

        /*
         * A valid user in another domain - return OK for this.
         */
        if (!userJid.getDomain().equals(this.getXmppDomain()))
            return true;
        try {
            String username = userJid.getNode();
            this.userManager.getUser(username);
        } catch (UserNotFoundException e) {
            return false;
        }
        return true;
    }

}
