package org.sipfoundry.openfire.client;

import java.net.URL;
import java.util.Map;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.XmlRpcPresenceProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcUserAccountProvider;

public class OpenfireXmlRpcUserAccountClient extends OpenfireXmlRpcClient {
    private String serverAddress;
    private boolean isSecure;
    private XmlRpcClient client;
    private String server;

    public OpenfireXmlRpcUserAccountClient(String serverAddress, int port) throws Exception {
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setEnabledForExceptions(true);
        config.setEnabledForExtensions(true);
        this.serverAddress = serverAddress;
        String url = (isSecure ? "https" : "http") + "://" + serverAddress + ":" + port+"/plugins/sipx-openfire/" + XmlRpcUserAccountProvider.SERVICE;
        System.out.println("URL = " + url);
        config.setServerURL(new URL(url));
        this.client = new XmlRpcClient();
        this.client.setConfig(config);
        this.server = XmlRpcUserAccountProvider.SERVER;
    }
    
    public boolean userExists(String userName) throws OpenfireClientException {
       
        Object[] args = new Object[1];
        args[0] = userName;
        Map retval;
        try {
            retval = (Map) client.execute(server + ".userExists" , args  );
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
            retval = (Map) client.execute(server + ".createUserAccount",args);
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
            retval = (Map) client.execute(server + ".destroyUserAccount", args);
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
            retval = (Map) client.execute(server + ".setSipId", args);
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
            retval = (Map) client.execute(server + ".getSipId",args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return (String) retval.get(XmlRpcPresenceProvider.SIP_ID);
    }
    
    public void setSipPassword(String userName, String sipPassword) throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = userName ;
        args[1] = sipPassword;
        Map retval;
        try {
            retval = (Map) client.execute(server + ".setSipPassword", args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }
    
    public String getSipPassword(String userName) throws OpenfireClientException {
        Object[] args = new Object[1];
        args[0] = userName;
        Map retval;
        try {
            retval = (Map) client.execute(server + ".getSipPassword",args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
        return (String) retval.get(XmlRpcPresenceProvider.SIP_PASSWORD);
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
            retval = (Map) client.execute(server + ".setOnThePhoneMessage" , args);
        } catch ( XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
       
        if (retval.get(XmlRpcPresenceProvider.STATUS_CODE).equals(XmlRpcPresenceProvider.ERROR)) {
            throw new OpenfireClientException("Error in processing request "
                    + retval.get(XmlRpcPresenceProvider.ERROR_INFO));
        }
    }

    public String getOnThePhoneMessage(String sipUser) throws OpenfireClientException {
       Object[] args = new Object[1];
       args[0] = sipUser;
       Map retval ;
       
       try {
           retval = (Map) client.execute(server + ".getOnThePhoneMessage" , args);
           return (String) retval.get(XmlRpcProvider.ON_THE_PHONE_MESSAGE);
       } catch ( XmlRpcException ex) {
           throw new OpenfireClientException(ex);
       }
       
    }
    
    
    
    
}
