package org.sipfoundry.openfire.plugin.presence;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
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
import org.jivesoftware.openfire.muc.MUCRole.Role;
import org.jivesoftware.openfire.spi.PresenceManagerImpl;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserAlreadyExistsException;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.user.UserNotFoundException;
import org.jivesoftware.util.AlreadyExistsException;
import org.jivesoftware.util.JiveGlobals;
import org.jivesoftware.util.NotFoundException;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.openfire.config.AccountsParser;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.openfire.config.XmppAccountInfo;
import org.sipfoundry.sipcallwatcher.CallWatcher;
import org.sipfoundry.sipcallwatcher.ProtocolObjects;
import org.sipfoundry.sipcallwatcher.ResourceStateChangeListener;
import org.xmpp.component.Component;
import org.xmpp.component.ComponentManager;
import org.xmpp.component.ComponentManagerFactory;
import org.xmpp.packet.JID;
import org.xmpp.packet.Packet;
import org.xmpp.packet.Presence;

public class SipXOpenfirePlugin implements Plugin, Component {

    private static final String subdomain = "presence";
    
    
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
    
    private Map<String,String> roomNameToConferenceExtensionMap = new HashMap<String,String>();


    private AccountsParser accountsParser;

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
            logAppender = new SipFoundryAppender(new SipFoundryLayout(), logFile);
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

            applicationLogger.addAppender(logAppender);
            if (System.getProperty("output.console") != null) {
                applicationLogger.addAppender(new ConsoleAppender(new PatternLayout()));
            }

            CallWatcher.setLogAppender(logAppender);
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
        CallWatcher.setWatcherConfig(watcherConfig);
        /*
         * This initializes the SIP side of the show.
         */
        try {
            CallWatcher.pluginInit();
            log.info("completed init");
            ResourceStateChangeListener resourceStateChangeListener = new ResourceStateChangeListenerImpl(this);
            CallWatcher.getSubscriber().setResourceStateChangeListener(resourceStateChangeListener);
        } catch (Exception e) {
            log.error("Error initializing CallWatcher");
            throw new SipXOpenfirePluginException("Init error", e);
        }

        server = XMPPServer.getInstance();
       
        userManager = server.getUserManager();
        presenceManager = server.getPresenceManager();

        hostname = server.getServerInfo().getXMPPDomain();
        log.info("HostName = " + hostname);

        probedPresence = new ConcurrentHashMap<String, Presence>();
        componentJID = new JID(subdomain + "." + hostname);
        
        groupManager = GroupManager.getInstance();
        
        multiUserChatManager = server.getMultiUserChatManager();
        /*
         * Load up the database.
         */
        multiUserChatManager.start();

        log.info("hostname " + hostname);
        for (String userName : userManager.getUsernames()) {
            log.info("userName " + userName);
        }
          
        try {

            componentManager.addComponent(subdomain, this);

        } catch (Exception e) {
            componentManager.getLog().error(e);
            log.error(e);
            throw new SipXOpenfirePluginException("Init error", e);
        }
        for (User user : userManager.getUsers()) {
            String name = user.getUsername() + "@" + this.getXmppDomain() ;
            log.info("Add user " + name);
            String sipId = user.getProperties().get(SIP_UID);
            if (sipId != null) {
                log.info("adding SIP ID " + sipId);
                this.sipIdToXmppIdMap.put(sipId, name);
            } else {
                log.info(" user has no sip ID ");
            }
            

        }
        
        // add packet interceptor
        InterceptorManager.getInstance().addInterceptor( new MessagePacketInterceptor(this) );
		 
        // config and instantiate and the presence unifier used to gather all presence info
        PresenceUnifier.setPlugin( this );
        PresenceUnifier.getInstance();
        String accountConfigurationFile = configurationPath + "/xmpp-account-info.xml";
        if ( !new File(accountConfigurationFile).exists()) {
           System.err.println("User account file not found");
        } else {
            this.accountsParser = new AccountsParser(accountConfigurationFile);
        }
    }

    public void destroyPlugin() {
        log.debug("DestroyPlugin");
        // Remove presence plugin component
        try {
            ProtocolObjects.stop();
        } catch (Exception e) {
            componentManager.getLog().error(e);
        }
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

    //Experimental - to be debugged and tested
    public Presence getForeignUserPresence(String sender, String jid) throws UserNotFoundException {

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

    public void setPresenceState(String jid, UnifiedPresence.XmppPresence xmppPresence) throws UserNotFoundException {
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
            String email)  {

        try {
            User user = userManager.getUser(userName);
            user.setPassword(password);
            user.setName(displayName);
            user.setEmail(email);
        } catch (UserNotFoundException e) {
            try {
                userManager.createUser(userName, password, displayName, email);
            } catch (UserAlreadyExistsException ex){
                throw new SipXOpenfirePluginException(ex);
            }
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
    
    
    public void createGroup(String groupName, String adminJid, String description) 
    throws IllegalArgumentException {
       log.debug("createGroup groupName = " + groupName);
       Group group = null;
       JID jid = new JID(adminJid); 
       if ( ! jid.getDomain().equals(this.getXmppDomain())) {
           throw new IllegalArgumentException("Domain must be local domain");
       }
       
       try {
           group = groupManager.getGroup(groupName);
       } catch ( GroupNotFoundException ex) {
           try {
               group = groupManager.createGroup(groupName);
               
           } catch ( GroupAlreadyExistsException ex1) {
               log.debug("Group already exists - not creating group");

           }
       }
       group.setDescription(description);
       group.getAdmins().remove(jid);
       group.getAdmins().add(jid);
      
    }

    public void addUserToGroup(String userJid,   String groupName) throws 
        GroupNotFoundException { 
        log.debug("addUserToGroup " + userJid + " GroupName " 
                + groupName );
        JID jid = new JID(userJid); 
        Group group = groupManager.getGroup(groupName,true);
        if ( group.getAdmins().contains(jid)) {
            log.debug("Admins already has " + jid);
        } else {
            group.getMembers().add(jid); 
        }
        
    }
    
    public void removeUserFromGroup(String userJid, String groupName) 
    throws GroupNotFoundException {
        JID jid = new JID(userJid);
        Group group = groupManager.getGroup(groupName,true);
        group.getMembers().remove(jid);
        group.getAdmins().remove(jid);
    }
    
    public void deleteGroup(String groupName) throws GroupNotFoundException {
        Group group = groupManager.getGroup(groupName);
        groupManager.deleteGroup(group);
        
    }
    
    public void setOnThePhoneMessage(String sipUserName, String onThePhoneMessage ) throws UserNotFoundException {
        String userName = this.sipIdToXmppIdMap.get(sipUserName);
        if ( userName == null ) {
            throw new UserNotFoundException("SIP User " + sipUserName + " not found");
        }
        User user = userManager.getUser(userName);
        if ( ! onThePhoneMessage.equals("")) {
            user.getProperties().put(ON_THE_PHONE_MESSAGE, onThePhoneMessage);
        } else {
            user.getProperties().remove(ON_THE_PHONE_MESSAGE);
        }
    }

    public String getOnThePhoneMessage(String sipUserName) throws UserNotFoundException {
        
        String userName = this.sipIdToXmppIdMap.get(sipUserName);
        if ( userName == null ) {
          throw new UserNotFoundException("User not found - no xmpp ID exists " + sipUserName);
        }
        User user = userManager.getUser(userName);
        return user.getProperties().get(ON_THE_PHONE_MESSAGE);
     
    }
    
    public void setConferenceExtension(String sipUserName, String conferenceExtension) throws UserNotFoundException {
        String userName = this.sipIdToXmppIdMap.get(sipUserName);
        if ( userName == null ) {
            throw new UserNotFoundException("SIP User " + sipUserName + " not found");
        }
        User user = userManager.getUser(userName);
        
        user.getProperties().put(CONFERENCE_EXTENSION, conferenceExtension);
        
        
    }
    
    public boolean groupExists(String groupName) {
        try {
            Group group = groupManager.getGroup(groupName);
            return true;
        } catch ( GroupNotFoundException ex) {
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
        Group group = groupManager.getGroup(groupName,true);
        return group.isUser(jid);
    }
    
   
    public void destroyMultiUserChatService(String subdomain) {
        try {
            this.multiUserChatManager.removeMultiUserChatService(subdomain);
          
        } catch (NotFoundException nfe) {
            log.debug("destroyMultiUserChatService not found "  + subdomain);
        }
    }
    
   
   
    public void createChatRoom(String domain,  
            String ownerJid,
            String roomName, 
            boolean listRoomInDirectory,
            boolean makeRoomModerated,
            boolean makeRoomMembersOnly,
            boolean allowOccupantsToInviteOthers,
            boolean isPublicRoom,
            boolean logRoomConversations,
            boolean isPersistent,
            String password,
            String description, 
            String conferenceExtension) 
    throws Exception {      
        MultiUserChatService mucService = XMPPServer.getInstance().getMultiUserChatManager().getMultiUserChatService(domain);
        if ( mucService == null ) {
            mucService = XMPPServer.getInstance().getMultiUserChatManager().createMultiUserChatService(domain,description,false);
            Collection<JID> admins = XMPPServer.getInstance().getAdmins();
            JID admin = admins.iterator().next();
            mucService.addUserAllowedToCreate(admin.toBareJID());
         
        }
        MUCRoom mucRoom = mucService.getChatRoom(roomName,new JID(ownerJid));
      
        mucRoom.setPersistent(isPersistent);
        mucRoom.setCanAnyoneDiscoverJID(true);
        mucRoom.setChangeNickname(true);
        mucRoom.setModerated(makeRoomModerated);
        mucRoom.setMembersOnly(makeRoomMembersOnly);
        mucRoom.setRegistrationEnabled(true);
        mucRoom.setPublicRoom(isPublicRoom);
        mucRoom.setCanAnyoneDiscoverJID(true);
        mucRoom.setCanOccupantsInvite(allowOccupantsToInviteOthers);
        mucRoom.setDescription(description);
        mucRoom.setPassword(password);
        mucRoom.setCanOccupantsChangeSubject(true);
        mucRoom.setChangeNickname(true);
        mucRoom.setLogEnabled(logRoomConversations);
       

        
        mucRoom.setDescription(description);
        
        mucRoom.setPassword(password);
        /* The conference extension is the voice conf bridge extension */
        this.roomNameToConferenceExtensionMap.put(domain+"."+roomName,conferenceExtension);
      
    }
    
    /**
     * Delete a chat room from the domain.
     * 
     * @param domain 
     * @param roomName
     */
    public void removeChatRoom(String domain, String roomName) {
        MultiUserChatService mucService = this.multiUserChatManager.getMultiUserChatService(domain);
        log.debug("removeChatRoom domain = " + domain + " roomName = " + roomName);
        mucService.removeChatRoom(roomName);
        this.roomNameToConferenceExtensionMap.remove(domain+ "." + roomName);
    }

    /**
     * Get all the members of a chat room.
     * 
     * @param domain
     * @param roomName
     * @return
     */
    public Collection<String> getMembers(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager.getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if ( mucRoom == null ){
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return mucRoom.getMembers();
     }

    public Map<String,String> getMucRoomAttributes(String domain, String roomName) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager.getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if ( mucRoom == null ){
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        Map<String,String> retval = new HashMap<String,String> ();
        retval.put("isModerated", ""+mucRoom.isModerated());
        retval.put("isLogEnabled", ""+mucRoom.isLogEnabled());
        retval.put("isMembersOnly", ""+mucRoom.isMembersOnly());
        retval.put("isPublicRoom",""+mucRoom.isPublicRoom());
        retval.put("isLoginRestrictedToNickName",""+mucRoom.isLoginRestrictedToNickname());
        retval.put("isLocked", ""+mucRoom.isLocked());
        retval.put("isRegistrationEnabled", ""+mucRoom.isRegistrationEnabled());
        retval.put("isPasswordProtected", ""+mucRoom.isPasswordProtected());
        retval.put("canAnyoneDiscoverJID", ""+mucRoom.canAnyoneDiscoverJID());
        retval.put("canChangeNickName", ""+mucRoom.canChangeNickname());
        retval.put("canOccupantsInvite", ""+mucRoom.canOccupantsInvite());
        retval.put("canOccupantsChangeSubject", ""+ mucRoom.canOccupantsChangeSubject());
        return retval;
    }

    public void setMucRoomAttributes(String domain, 
            String roomName, Map newAttributes) throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager.getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if ( mucRoom == null ){
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
         Map<String,String> attribs = (Map<String,String>) newAttributes;
         boolean isModerated = Boolean.parseBoolean(attribs.get("isModerated"));
         mucRoom.setModerated(isModerated);
         boolean isLogEnabled = Boolean.parseBoolean(attribs.get("isLogEnabled"));
         mucRoom.setLogEnabled(isLogEnabled);
         boolean isMembersOnly = Boolean.parseBoolean(attribs.get("isMembersOnly"));
         mucRoom.setMembersOnly(isMembersOnly);
         boolean isPublicRoom = Boolean.parseBoolean(attribs.get("isPublicRoom"));
         mucRoom.setPublicRoom(isPublicRoom);
         boolean isLoginRestrictedToNickName = Boolean.parseBoolean(attribs.get("isLoginRestrictedToNickName"));
         mucRoom.setLoginRestrictedToNickname(isLoginRestrictedToNickName);
         boolean isRegistrationEnabled = Boolean.parseBoolean(attribs.get("isRegistrationEnabled"));
         mucRoom.setRegistrationEnabled(isRegistrationEnabled);
         boolean canAnyoneDiscoverJID = Boolean.parseBoolean(attribs.get("canAnyoneDiscoverJID"));
         mucRoom.setCanAnyoneDiscoverJID(canAnyoneDiscoverJID);
         boolean canChangeNickName = Boolean.parseBoolean(attribs.get("canChangeNickName"));
         mucRoom.setChangeNickname(canChangeNickName);
         boolean canOccupantsInvite = Boolean.parseBoolean(attribs.get("canOccupantsInvite"));
         mucRoom.setCanOccupantsInvite(canOccupantsInvite);
         boolean canOccupantsChangeSubject = Boolean.parseBoolean(attribs.get("canOccupantsChangeSubject"));
         mucRoom.setCanOccupantsChangeSubject(canOccupantsChangeSubject);
    }
    
    
    public String getConferenceExtension(String domain, String roomName) 
    throws NotFoundException {
        MultiUserChatService mucService = this.multiUserChatManager.getMultiUserChatService(domain);
        MUCRoom mucRoom = mucService.getChatRoom(roomName);
        if ( mucRoom == null ){
            throw new NotFoundException("Room not found " + domain + " roomName " + roomName);
        }
        return this.roomNameToConferenceExtensionMap.get(domain + "." + roomName);
    }

    public HashSet<UserAccount> getUserAccounts() {
       HashSet<UserAccount> userAccounts = new HashSet<UserAccount>();
       for ( User user : this.userManager.getUsers() ){
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
        for (MultiUserChatService mucService : 
            this.multiUserChatManager.getMultiUserChatServices()) {
            List<MUCRoom> chatRooms = mucService.getChatRooms();
            retval.addAll(chatRooms);
        }
        return retval;
    }
   
    
   

}
