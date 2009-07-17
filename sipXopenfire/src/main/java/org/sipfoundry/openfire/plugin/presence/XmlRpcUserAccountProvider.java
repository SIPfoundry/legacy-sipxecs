package org.sipfoundry.openfire.plugin.presence;

import java.util.Map;

import org.apache.log4j.Logger;

public class XmlRpcUserAccountProvider extends XmlRpcProvider {

    public static final String SERVICE = "user";

    public static final String SERVER = "userAccountServer";

    private static Logger log = Logger.getLogger(XmlRpcUserAccountProvider.class);

    public XmlRpcUserAccountProvider() {
    }

    public Map userExists(String userName) {
        userName = appendDomain(userName);

        log.debug("userExists " + userName);
        Map retval = createSuccessMap();
        boolean hasAccount = plugin.userHasAccount(userName);
        retval.put(ACCOUNT_EXISTS, new Boolean(hasAccount).toString());
        log.debug("hasAccount = " + hasAccount);
        return retval;
    }

    public Map createUserAccount(String userName, String password, String displayName,
            String email) {

        try {
            // userName = setDomain(userName);
            log.debug(String.format("createUserAccount %s,%s,%s,%s", userName, password,
                    displayName, email));
            plugin.createUserAccount(userName, password, displayName, email);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.USER_CREATION_ERROR, ex.getMessage());
        }
    }

    public Map destroyUserAccount(String userName) {
        try {
            String jid = appendDomain(userName);
            log.debug("destroyUserAccount " + jid);
            plugin.destroyUser(jid);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.USER_DESTRUCTION_ERROR, ex.getMessage());
        }
    }

    public Map setSipId(String userName, String sipUserName) {
        try {
            String jid = appendDomain(userName);
            log.debug("setSipId " + jid + " sipUserName " + sipUserName);
            plugin.setSipId(jid, sipUserName);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SET_SIP_ID_ERROR, ex.getMessage());
        }
    }

    /**
     * Get SIP ID corresponing to a xmpp userName ( for this domain ).
     * 
     * @param userName
     * @return
     */
    public Map getSipId(String userName) {
        try {
            String sipId = plugin.getSipId(userName);
            Map retval = createSuccessMap();
            if (sipId != null) {
                retval.put(SIP_ID, sipId);
            }
            return retval;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GET_SIP_ID_ERROR, ex.getMessage());
        }
    }

    public Map setSipPassword(String userName, String sipPassword) {
        try {
            String jid = appendDomain(userName);
            //TODO: remove SIP password from logs!!!
            log.debug("setSipPassword " + jid + " sipPassword " + sipPassword);
            plugin.setSipPassword(jid, sipPassword);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SET_SIP_PASSWORD_ERROR, ex.getMessage());
        }
    }

    /**
     * Get SIP Password corresponing to a xmpp userName ( for this domain ).
     * 
     * @param userName
     * @return
     */
    public Map getSipPassword(String userName) {
        try {
            String sipPassword = plugin.getSipPassword(userName);
            Map retval = createSuccessMap();
            if (sipPassword != null) {
                retval.put(SIP_PASSWORD, sipPassword);
            }
            return retval;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GET_SIP_PASSWORD_ERROR, ex.getMessage());
        }
    }

    /**
     * Set the onThePhone message for this sip ID
     * 
     * @param sipUser : The sipUserName@sipDomain
     */
    public Map setOnThePhoneMessage(String sipUser, String onThePhoneMessage) {
        try {
            plugin.setOnThePhoneMessage(sipUser, onThePhoneMessage);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SIP_ID_NOT_FOUND, ex.getMessage());
        }
    }

    /**
     * Get on the phone message for this sip ID.
     * 
     * @param sipUser : the sip userName@sip-domain
     */

    public Map getOnThePhoneMessage(String sipUser) {
        try {
            String onThePhoneMessage = plugin.getOnThePhoneMessage(sipUser);
            Map map = createSuccessMap();
            if (onThePhoneMessage != null) {
                map.put(ON_THE_PHONE_MESSAGE, onThePhoneMessage);
            }
            return map;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SIP_ID_NOT_FOUND, ex.getMessage());
        }
    }

}
