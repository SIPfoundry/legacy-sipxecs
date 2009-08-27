package org.sipfoundry.openfire.plugin.presence;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import org.apache.log4j.Logger;

public class XmlRpcUserAccountProvider extends XmlRpcProvider {

   
   
    private static Logger log = Logger.getLogger(XmlRpcUserAccountProvider.class);
    
    public final static String SERVICE = "user";

    public final static String SERVER = "userAccountServer";
    

    public XmlRpcUserAccountProvider() {
    }

    public Map userExists(String userName) {
        userName = appendDomain(userName);

        log.debug("userExists " + userName);
        Map retval = createSuccessMap();
        boolean hasAccount = getPlugin().userHasAccount(userName);
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
            getPlugin().createUserAccount(userName, password, displayName, email);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.USER_CREATION_ERROR, ex.getMessage());
        }
    }

    public Map destroyUserAccount(String userName) {
        try {
            String jid = appendDomain(userName);
            log.debug("destroyUserAccount " + jid);
            getPlugin().destroyUser(jid);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.USER_DESTRUCTION_ERROR, ex.getMessage());
        }
    }

    public Map setSipId(String userName, String sipUserName) {
        try {
            String jid = appendDomain(userName);
            log.debug("setSipId " + jid + " sipUserName " + sipUserName);
            getPlugin().setSipId(jid, sipUserName);
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
            String sipId = getPlugin().getSipId(userName);
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
            getPlugin().setSipPassword(jid, sipPassword);
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
            String sipPassword = getPlugin().getSipPassword(userName);
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
            getPlugin().setOnThePhoneMessage(sipUser, onThePhoneMessage);
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
            String onThePhoneMessage = getPlugin().getOnThePhoneMessage(sipUser);
            Map map = createSuccessMap();
            if (onThePhoneMessage != null) {
                map.put(ON_THE_PHONE_MESSAGE, onThePhoneMessage);
            }
            return map;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SIP_ID_NOT_FOUND, ex.getMessage());
        }
    }
    
    /**
     * Create a group.
     * 
     * @param groupName -- the group name.
     */
    
    public Map createGroup(String groupName, String administrator, String groupDescription ) {
        try {
            String adminJid = appendDomain(administrator);
            getPlugin().createGroup(groupName, adminJid, groupDescription);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GROUP_CREATION_ERROR,ex.getMessage());
        }
    }
    
    /**
     * Delete a group.
     * 
     * @param groupName -- the group name.
     */
    public Map deleteGroup(String groupName ) {
        try {
            getPlugin().deleteGroup(groupName);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GROUP_DELETION_ERROR,ex.getMessage());
        }
    }
    
    /**
     * Add a user to a group.
     * 
     * @param userName -- user name to add.
     
     * @param groupName -- group name.
     * 
     * @param isAdmin -- flag that indicates whether or not user is an admin.
     * 
     */
    public Map addUserToGroup(String userName, String groupName) {
        try {
            String userJid = appendDomain(userName);
            getPlugin().addUserToGroup(userJid, groupName);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.ADD_USER_TO_GROUP_ERROR,ex.getMessage());
        }
    }
    
    /**
     * Remove user from group.
     * 
     * @param userName -- user name to remove.
     * @param groupName -- group name to remove user from.
     * 
     */
    public Map removeUserFromGroup(String userName, String groupName) {
        try {
            String userJid = appendDomain(userName);
            getPlugin().removeUserFromGroup(userJid, groupName);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.REMOVE_USER_FROM_GROUP,ex.getMessage());
        }
    }
    
    
    /**
     * Return true if User is in group
     * 
     * @param userName  -- user name to test.
     * @param groupName -- group name to test membership.
     */
    public Map isUserInGroup(String userName, String groupName) {
        try {
            String userJid = appendDomain(userName);
            boolean result = getPlugin().isUserInGroup( userName, groupName);
            Map retval = createSuccessMap();
            retval.put(XmlRpcProvider.USER_IN_GROUP, new Boolean(result).toString());
            return retval;      
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.USER_IN_GROUP_EXCEPTION, ex.getMessage());
        }
    }
    
    
    
    /**
     * Return true if a  group exists.
     */
    public Map groupExists(String groupName) {
        try {
            boolean exists = getPlugin().groupExists(groupName);
            Map map = createSuccessMap();
            map.put(GROUP_EXISTS, new Boolean(exists).toString());
            return map;
        } catch(Exception ex) {
            return createErrorMap(ErrorCode.GROUP_EXISTS_ERROR,ex.getMessage());
        }
    }
    
    
    public Map getUserAccounts() {
        try {
            HashSet<UserAccount> userAccounts = getPlugin().getUserAccounts();
            Map retval = createSuccessMap();
            Map<String,String>[] userAccountArray = new HashMap[userAccounts.size()];
            int j = 0;
            for (UserAccount userAccount : userAccounts ) {
                HashMap<String,String> userAccountMap = new HashMap<String,String>();
                userAccountMap.put(XMPP_USER_NAME, userAccount.getXmppUserName());
                if ( userAccount.getSipUserName() != null ) {
                    userAccountMap.put(SIP_ID, userAccount.getSipUserName());
                }
                userAccountArray[j++] = userAccountMap;               
            }
            retval.put(USER_ACCOUNTS, userAccountArray);
            return retval;
        } catch(Exception ex) {
            return createErrorMap(ErrorCode.GROUP_EXISTS_ERROR,ex.getMessage());
        }
    }
    
    
   
   
    
 
    

}
