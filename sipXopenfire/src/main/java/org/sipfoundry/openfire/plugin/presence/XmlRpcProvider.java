package org.sipfoundry.openfire.plugin.presence;

import java.util.HashMap;
import java.util.Map;

import org.xmpp.packet.Presence;

public abstract class XmlRpcProvider {
    public static final String STATUS_CODE = "status-code";

    public static final String ERROR = "error";

    public static final String OK = "ok";

    public static final String ERROR_CODE = "error-code";

    public static final String ERROR_INFO = "error-info";

    public static final String XMPP_PRESENCE = "xmpp-presence";
    
    public static final String CUSTOM_PRESENCE_MESSAGE = "custom-presence-message";
    
    public static final String ACCOUNT_EXISTS = "account-exists";
    
    public static final String SIP_ID = "sipId";
    
    public static final String SIP_PASSWORD = "sipPassword";
    
    public static final String JABBER_ID = "jabber-id";
    
    public static final String SIP_PRESENCE = "sip-presence";
    
    public static final String UNIFIED_PRESENCE = "unified-presence";
    
    public static final String ON_THE_PHONE_MESSAGE = "on-the-phone-messasge";

    protected static SipXOpenfirePlugin plugin;
    
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
            return userName + "@" + plugin.getXmppDomain();
        } else {
            return userName;
        }
    }
}
