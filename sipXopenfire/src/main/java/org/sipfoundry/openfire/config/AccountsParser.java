package org.sipfoundry.openfire.config;

import java.io.File;
import java.util.Collection;
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
    public static String groupMember = String.format("%s/%s", groupTag, USER);

    private static String currentTag = null;
    private static Digester digester;
    private String accountDbFileName;
    private File accountDbFile;
    private long lastModified;
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
                    String fileUrl = "file://" + accountDbFileName;
                    XmppAccountInfo accountInfo = AccountsParser.this.parse(fileUrl);
                    SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();
                    Collection<UserAccount> userAccounts = new HashSet<UserAccount>();
                    userAccounts.addAll(plugin.getUserAccounts());
                    
                    for (UserAccount userAccount : userAccounts) {
                        if (accountInfo.getXmppUserAccount(userAccount.getXmppUserName()) == null) {
                            if ( ! userAccount.getXmppUserName().equals("admin")) {
                                plugin.destroyUser(XmppAccountInfo.appendDomain(userAccount.getXmppUserName()));
                            }
                        }
                    }
                    HashSet<Group> groups = new HashSet<Group>();
                    groups.addAll(plugin.getGroups());
                    for (Group group : groups) {
                        if ( accountInfo.getXmppGroup(group.getName()) == null ) {
                            logger.debug("Cannot find group in config directory " + group.getName());
                            plugin.deleteGroup(group.getName());
                            continue;
                        }
                        XmppGroup xmppGroup = accountInfo.getXmppGroup(group.getName());
                        Collection<JID> members = group.getMembers();
                        for (JID member : members) {
                            /* Make sure we have a record for this member 
                             * in our sipxconfig generated group.
                             */
                            if ( !xmppGroup.hasMember(member.toBareJID())) {
                                group.getMembers().remove(member);
                            }
                            
                        }
                    }
                    Collection<MUCRoom> chatRooms = plugin.getMUCRooms();
                    /*
                     * Remove any chat rooms that do not appear in the xml file.
                     */
                    for (MUCRoom mucRoom : chatRooms) {
                        String domain = mucRoom.getMUCService()
                        .getServiceDomain().split("\\.")[0];
                        logger.debug("Checking MUCRoom" + domain);
                        XmppChatRoom xmppChatRoom = accountInfo.getXmppChatRoom(domain, mucRoom.getName());
                        if (xmppChatRoom == null) {
                            logger.debug("Remove MUCRoom " + mucRoom.getName());
                            plugin.removeChatRoom(domain,mucRoom.getName());
                           
                        } 
                    }
                    
                    /*
                     * Restrict the chat services to those that are in xmpp-account-info.xml
                     */
                    HashSet<String>allowedDomains = new HashSet<String>();
                    for ( XmppChatRoom chatRoom: accountInfo.getXmppChatRooms()) {
                        allowedDomains.add(chatRoom.getSubdomain());
                    }
                    plugin.pruneChatServices(allowedDomains);
                    
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
        digester.addObjectCreate(groupMember, XmppGroupMember.class.getName());
        digester.addSetNext(groupMember, "addMember");
        digester.addObjectCreate(chatRoomTag, XmppChatRoom.class.getName());
        digester.addSetNext(chatRoomTag, "addChatRoom");
        currentTag = userTag;
        addCallMethod("password", "setPassword");
        addCallMethod("user-name", "setUserName");
        addCallMethod("sip-user-name", "setSipUserName");
        addCallMethod("display-name", "setDisplayName");
        addCallMethod("on-the-phone-message", "setOnThePhoneMessage");
        addCallMethod("conference-extension", "setConferenceExtension");
        currentTag = groupTag;
        addCallMethod("group-name", "setGroupName");
        addCallMethod("description", "setDescription");
        addCallMethod("administrator", "setAdministrator");
        currentTag = groupMember;
        addCallMethod("user-name", "setUserName");
        currentTag = chatRoomTag;
        addCallMethod("subdomain", "setSubdomain");
        addCallMethod("conference-extension", "setConferenceExtension");
        addCallMethod("room-name", "setRoomName");
        addCallMethod("description", "setDescription");
        addCallMethod("password", "setPassword");
        addCallMethod("room-owner", "setOwner");
        addCallMethod("log-room-conversations","setLogRoomConversations");
        addCallMethod("is-public-room","setIsPublicRoom");
        addCallMethod("is-members-only", "setMembersOnly");
        addCallMethod("is-persistant","setPersistant");
        addCallMethod("is-room-listed","setRoomListed");

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
