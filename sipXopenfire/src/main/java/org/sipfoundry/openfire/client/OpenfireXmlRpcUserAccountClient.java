package org.sipfoundry.openfire.client;

import java.net.URL;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.UserAccount;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcUserAccountProvider;

public class OpenfireXmlRpcUserAccountClient extends OpenfireXmlRpcClient {
   
    public OpenfireXmlRpcUserAccountClient(String serverAddress, int port) throws Exception {
       super(XmlRpcUserAccountProvider.SERVER, XmlRpcUserAccountProvider.SERVICE, serverAddress,port);
    }
    
    public boolean userExists(String userName) throws OpenfireClientException {
       
        Object[] args = new Object[1];
        args[0] = userName;
        Map retval;
        try {
            retval = (Map) execute( "userExists" , args  );
        } catch (XmlRpcException e) {
          throw new OpenfireClientException(e);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return retval.get(XmlRpcProvider.ACCOUNT_EXISTS).equals("true");
        
    }
    
    public void createUserAccount(String userName, String password, 
            String sipUserName, String email) throws OpenfireClientException {
        Object[] args = new Object[4];
        args[0] = userName;
        args[1] = password;
        args[2] = sipUserName;
        args[3] = email;
        Map retval ;
        
        try {
            retval = (Map) execute("createUserAccount",args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    /**
     * This is a placeholder. This method does not work on the server.
     * @param userName
     * @throws OpenfireClientException
     */
    public void destroyUserAccount(String userName) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = userName;
        Map retval;
        try {
            retval = execute("destroyUserAccount", args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    
    public void setSipId(String userName, String sipId) throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = userName ;
        args[1] = sipId;
        Map retval;
        try {
            retval = execute("setSipId", args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    
    public String getSipId(String userName) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = userName;
        Map retval;
        try {
            retval = execute("getSipId",args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return (String) retval.get(XmlRpcPresenceProvider.SIP_ID);
    }
    
   
    
    public void setOnThePhoneMessage(String sipUserName, String onThePhoneMessage) throws OpenfireClientException {
        Object[] args = new Object[2];
        if ( onThePhoneMessage == null ) {
            onThePhoneMessage = "";
        }
        if ( sipUserName.indexOf("@") == -1) {
            throw new OpenfireClientException("Invalid sip user name provided " + sipUserName + " must be user@domain");
        } 
        
        args[0] = sipUserName;
        args[1] = onThePhoneMessage ;
        
        
        Map retval ;
        
        try {
            retval = execute("setOnThePhoneMessage" , args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
       
       
    }

    public String getOnThePhoneMessage(String sipUser) throws OpenfireClientException {
       Object[] args = new Object[1];
       args[0] = sipUser;
       Map retval ;
       
       try {
           retval = execute("getOnThePhoneMessage" , args);
           return (String) retval.get(XmlRpcProvider.ON_THE_PHONE_MESSAGE);
       } catch ( XmlRpcException ex) {
           throw new OpenfireClientException(ex);
       }
       
    }
    
    public void createGroup(String groupName, String admin, String description) throws OpenfireClientException {
        Object[] args = new Object[3];
        args[0] = groupName;
        args[1] = admin;
        args[2] = description;
        Map retval ;
        
        try {
            retval = execute("createGroup",args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    public void deleteGroup(String groupName) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = groupName;
        try {
            execute("deleteGroup",args); 
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    public void addUserToGroup(String userName, String groupName) 
    throws OpenfireClientException  {
        Object[] args = new Object[2];
        args[0] = userName;
        args[1] = groupName;
        
        try {
            execute("addUserToGroup", args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    public boolean groupExists(String groupName) 
    throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = groupName;
        try {
            Map map = (Map) execute("groupExists",args);
            return Boolean.parseBoolean((String)map.get(XmlRpcProvider.GROUP_EXISTS));
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }

    public boolean isUserInGroup(String userName, String groupName)
    throws OpenfireClientException{
       Object[] args = new Object[2];
       args[0] = userName;
       args[1] = groupName;
       try {
           Map retval = execute("isUserInGroup" , args);
           return Boolean.parseBoolean((String)retval.get(XmlRpcProvider.USER_IN_GROUP));
       } catch ( XmlRpcException ex) {
           throw new OpenfireClientException(ex);
       }
        
    }

    public void removeUserFromGroup(String userName, String groupName) 
    throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = userName;
        args[1] = groupName;
        try {
            Map retval = execute("removeUserFromGroup", args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }   
    }
    
    public Set<UserAccount> getUserAccounts()  throws OpenfireClientException {
       
        try {
            Map retval = execute("getUserAccounts",null);
            Set<UserAccount> userAccounts = new HashSet<UserAccount>();
            Object[] accounts = (Object[]) retval.get(XmlRpcProvider.USER_ACCOUNTS);
            for (Object accountMap : accounts) {
            	Map account = ( Map<String,Object> ) accountMap;
                UserAccount ua = new UserAccount();
                ua.setSipUserName((String)account.get(XmlRpcProvider.SIP_ID));
                ua.setXmppUserName((String)account.get(XmlRpcProvider.XMPP_USER_NAME));
                userAccounts.add(ua);
            }
            return userAccounts;
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
  
    
}
