package org.sipfoundry.openfire.plugin.presence;

import java.util.HashMap;
import java.util.Map;

import org.jivesoftware.openfire.XMPPServer;

public abstract class XmlRpcProvider {
    public static final String STATUS_CODE = "status-code";

    public static final String ERROR = "error";

    public static final String OK = "ok";

    public static final String ERROR_CODE = "error-code";

    public static final String ERROR_INFO = "error-info";

    public static final String XMPP_PRESENCE = "xmpp-presence";
    
    public static final String CUSTOM_PRESENCE_MESSAGE = "custom-presence-message";
    
    public static final String ACCOUNT_EXISTS = "account-exists";
    
    public static final String SIP_ID = "sip-id";
    
    public static final String SIP_PASSWORD = "sip-password";
    
    public static final String JABBER_ID = "jabber-id";
    
    public static final String SIP_PRESENCE = "sip-presence";
    
    public static final String UNIFIED_PRESENCE = "unified-presence";
    
    public static final String INSTANCE_HANDLE = "instance-handle";
    
    public static final String ON_THE_PHONE_MESSAGE = "on-the-phone-messasge";
    
    public static final String GROUP_EXISTS = "group-exists";

    public static final String USER_IN_GROUP = "is-user-in-group";
    
    public static final String ROOM_MEMBERS = "room-members";
    
    public static final String IS_LIST_ROOM_IN_DIRECTORY = "is-list-room-in-directory";

    public static final String CONFERENCE_BRIDGE_EXTENSION = "conference-bridge-extension";
    
    public static final String SIP_USER_NAME = "sip-user-name";
    
    public static final String XMPP_USER_NAME = "xmpp-user-name";
    
    public static final String USER_ACCOUNTS = "user-accounts";
    
    private static SipXOpenfirePlugin plugin ;
    
    
    protected Map<String, Object> createErrorMap(ErrorCode errorCode, String reason) {
        Map<String, Object> retval = new HashMap<String, Object>();
        retval.put(STATUS_CODE, ERROR);
        retval.put(ERROR_CODE, errorCode.toString());
        retval.put(ERROR_INFO, reason);
        return retval;
    }

    protected Map<String, Object> createSuccessMap() {
        Map<String, Object> retval = new HashMap<String, Object>();
        retval.put(STATUS_CODE, OK);
        return retval;
    }
    
    protected static String appendDomain(String userName) {
        if (  userName.indexOf("@") == -1) {
            // No @ in the domain so assume this is our domain.
            return userName + "@" + getPlugin().getXmppDomain();
        } else {
            return userName;
        }
    }


    /**
     * @return the plugin
     */
    protected static SipXOpenfirePlugin getPlugin() {
        return   (SipXOpenfirePlugin) XMPPServer.getInstance().getPluginManager().getPlugin("sipx-openfire");
    }
}
