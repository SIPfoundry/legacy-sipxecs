package org.sipfoundry.openfire.config;

import java.io.File;
import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.commons.digester.Digester;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.jivesoftware.openfire.PacketRouter;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.muc.MUCRole;
import org.jivesoftware.openfire.muc.MUCRoom;
import org.jivesoftware.openfire.muc.MultiUserChatService;
import org.jivesoftware.openfire.muc.spi.LocalMUCRole;
import org.jivesoftware.openfire.muc.spi.LocalMUCRoom;
import org.jivesoftware.openfire.muc.spi.LocalMUCUser;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;
import org.sipfoundry.openfire.plugin.presence.UserAccount;
import org.xml.sax.InputSource;
import org.xmpp.packet.JID;
import org.xmpp.packet.Presence;

public class AccountsParser {

    public static final String XMPP_INFO = "xmpp-account-info";
    public static final String USER = "user";
    public static final String GROUP = "group";
    public static final String CHAT_ROOM = "chat-room";
    public static String userTag = String.format("%s/%s", XMPP_INFO, USER);
    public static String groupTag = String.format("%s/%s", XMPP_INFO, GROUP);
    public static String chatRoomTag = String.format("%s/%s", XMPP_INFO, CHAT_ROOM);
    public static String groupMemberTag = String.format("%s/%s", groupTag, USER);

    private static String currentTag = null;
    private static Digester digester;
    private String accountDbFileName;
    private long lastModified;
    private File accountDbFile;
    private static Logger logger = Logger.getLogger(AccountsParser.class);
  
    private static Timer timer = new Timer();

    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger("org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
    }

    class Scanner extends TimerTask {

        @Override
        public void run() {
            try {
                if (accountDbFile.lastModified() != AccountsParser.this.lastModified) {
                    logger.info("XMPP account configuration changes detected - reparsing file");
                    AccountsParser.this.lastModified = accountDbFile.lastModified(); 
                    String fileUrl = "file://" + accountDbFileName;
                    XmppAccountInfo accountInfo = AccountsParser.this.parse(fileUrl);
                    SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
                    Collection<UserAccount> userAccounts = new HashSet<UserAccount>();
                    userAccounts.addAll(plugin.getUserAccounts());
    
                    for (UserAccount userAccount : userAccounts) {
                        if (accountInfo.getXmppUserAccount(userAccount.getXmppUserName()) == null) {
                            if (!userAccount.getXmppUserName().equals("admin")) {
                                plugin.destroyUser(XmppAccountInfo.appendDomain(userAccount
                                        .getXmppUserName()));
                            }
                        }
                    }
                    HashSet<Group> groups = new HashSet<Group>();
                    groups.addAll(plugin.getGroups());
                    for (Group group : groups) {
                        if (accountInfo.getXmppGroup(group.getName()) == null) {
                            logger.debug("Cannot find group in config directory " + group.getName());
                            plugin.deleteGroup(group.getName());
                            continue;
                        }
                    }
                    Collection<MUCRoom> chatRooms = plugin.getMUCRooms();
                    /*
                     * Remove any chat rooms that do not appear in the xml file.
                     */
                    for (MUCRoom mucRoom : chatRooms) {
                        String domain = mucRoom.getMUCService().getServiceDomain().split("\\.")[0];
                        logger.debug("Checking MUCRoom" + domain + " name " + mucRoom.getName());
                        XmppChatRoom xmppChatRoom = accountInfo.getXmppChatRoom(domain, mucRoom
                                .getName());
                        // Found a chat room that is not in our list of chat rooms.
    
                        if (xmppChatRoom == null) {
                            if (!mucRoom.wasSavedToDB()) {
                                /*
                                 * Anything not in our database is deemed adhoc.
                                 */
                                logger.debug("Adhoc chat room detected - enable logging");
                                mucRoom.setLogEnabled(true);
                                logger.debug("AdHoc chat room has owners : " + mucRoom.getOwners());
                                mucRoom.setLogEnabled(true);
                            } else {
                                logger.debug("Destroy chat room " + mucRoom.getName());
                                mucRoom.destroyRoom(null, "not a managed chat");
    
                            }
                        }
                    }
    
                    /*
                     * Restrict the chat services to those that are in xmpp-account-info.xml
                     */
                    HashSet<String> allowedDomains = new HashSet<String>();
                    for (XmppChatRoom chatRoom : accountInfo.getXmppChatRooms()) {
                        allowedDomains.add(chatRoom.getSubdomain());
                    }
                    plugin.pruneChatServices(allowedDomains);
                    /*
                     * Make sure that all user accounts can create multi-user chatrooms
                     */
                    plugin.setAllowedUsersForChatServices(plugin.getUserAccounts());
                }
            } catch (Exception ex) {
                logger.error("Exception caught while parsing accountsdb ", ex);
            }

        }

    }

    public AccountsParser(String accountDbFileName) {
        try {
            digester = new Digester();
            addRules(digester);
        } catch (Exception ex) {
            throw new SipXOpenfirePluginException(ex);
        }

        this.accountDbFileName = accountDbFileName;
        this.accountDbFile = new File(accountDbFileName);
        if (!accountDbFile.exists()) {
            throw new SipXOpenfirePluginException("Account db file not found : "
                    + accountDbFileName);
        }

    }

    public void startScanner() {
        Scanner scanner = new Scanner();
        timer.schedule(scanner, 0, 10000);
    }

    public void stopScanner() {
        timer.cancel();
    }

    private static void addCallMethod(String elementName, String methodName) {
        digester.addCallMethod(String.format("%s/%s", currentTag, elementName), methodName, 0);
    }

    /*
     * Add the digester rules.
     * 
     * @param digester
     */
    private static void addRules(Digester digester) throws Exception {
        AccountsParser.digester = digester;
        digester.setUseContextClassLoader(true);
        digester.addObjectCreate(XMPP_INFO, XmppAccountInfo.class.getName());

        digester.addObjectCreate(userTag, XmppUserAccount.class.getName());
        digester.addSetNext(userTag, "addAccount");

        digester.addObjectCreate(groupTag, XmppGroup.class.getName());
        digester.addSetNext(groupTag, "addGroup");

        digester.addObjectCreate(groupMemberTag, XmppGroupMember.class.getName());
        digester.addSetNext(groupMemberTag, "addMember");
        
        digester.addObjectCreate(chatRoomTag, XmppChatRoom.class.getName());
        digester.addSetNext(chatRoomTag, "addChatRoom");
        
        currentTag = userTag;
        addCallMethod("password", "setPassword");
        addCallMethod("user-name", "setUserName");
        addCallMethod("sip-user-name", "setSipUserName");
        addCallMethod("display-name", "setDisplayName");
        addCallMethod("on-the-phone-message", "setOnThePhoneMessage");
        addCallMethod("conference-extension", "setConferenceExtension");
        addCallMethod("advertise-on-call-status", "setAdvertiseOnCallPreference");
        addCallMethod("show-on-call-details", "setShowOnCallDetailsPreference");
        currentTag = groupTag;
        addCallMethod("group-name", "setGroupName");
        addCallMethod("description", "setDescription");
        addCallMethod("administrator", "setAdministrator");
        currentTag = groupMemberTag;
        addCallMethod("user-name", "setUserName");
        currentTag = chatRoomTag;
        addCallMethod("subdomain", "setSubdomain");
        addCallMethod("conference-extension", "setConferenceExtension");
        addCallMethod("conference-reach-info", "setConferenceReachabilityInfo");
        addCallMethod("room-name", "setRoomName");
        addCallMethod("description", "setDescription");
        addCallMethod("password", "setPassword");
        addCallMethod("room-owner", "setOwner");
        addCallMethod("moderated", "setModerated");
        addCallMethod("log-room-conversations", "setLogRoomConversations");
        addCallMethod("is-public-room", "setIsPublicRoom");
        addCallMethod("is-members-only", "setMembersOnly");
        addCallMethod("is-persistent", "setPersistent");

    }

    public XmppAccountInfo parse(String url) {
        // Create a Digester instance

        try {

            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            XmppAccountInfo accountInfo = (XmppAccountInfo) digester.getRoot();

            return accountInfo;
        } catch (Exception ex) {

            System.err.println(ex);
            throw new SipXOpenfirePluginException(ex);
        }

    }

}
