package org.sipfoundry.openfire.client;

import java.util.Map;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.sipfoundry.openfire.plugin.presence.XmlRpcChatRoomManagementProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcUserAccountProvider;

public class OpenfireXmlRpcChatRoomManagementClient extends OpenfireXmlRpcClient {
    
    
    public OpenfireXmlRpcChatRoomManagementClient(String serverAddress, int port) throws Exception {
       super(XmlRpcChatRoomManagementProvider.SERVER, XmlRpcChatRoomManagementProvider.SERVICE, serverAddress,port);
    }
    
    public void createChatRoom(String subdomain, String chatRoomName,        
            String description, 
            String password, 
            String conferenceBridgeExtension)
    throws OpenfireClientException {
        Object[] args = new Object[5];
        args[0] = subdomain;
        args[1] = chatRoomName;
        args[2] = description == null ? "" : description;
        args[3] = password;
        args[4] = conferenceBridgeExtension == null ? "" : conferenceBridgeExtension;
        try {
            Map retval = execute("createChatRoom",args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }      
    }
    
    public void removeChatRoom(String subdomain, String chatRoomName)
    throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = subdomain;
        args[1] = chatRoomName;
        try {
            execute("removeChatRoom", args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    
    public void destroyMultiUserChatService(String subdomain) 
    throws OpenfireClientException {
        Object[] args = new Object[1];
        try {
            Map retval = execute("destroyMultiUserChatService",args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    public String[] getMembers(String subdomain, String roomName) 
      throws OpenfireClientException {
        Object[] args  = new Object[2];
        args[0] = subdomain;
        args[1] = roomName;
        try {
            Map reval = execute("getMembers", args);
            Object[] mem = (Object[]) reval.get(XmlRpcChatRoomManagementProvider.ROOM_MEMBERS);
            String[] retval = new String[mem.length];
            
            for ( int i = 0 ; i < mem.length ; i++ ) {
                retval[i] = mem[i].toString(); 
            }
            return retval;
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        
    }
    
    
    /**
     * Following are the room attributes that are returned.
     * 
     * isModerated
     * isLogEnabled
     * isMembersOnly
     * isPublicRoom
     * isLoginRestrictedToNickName
     * isLocked
     * isRegistrationEnabled
     * isPasswordProtected
     * canAnyoneDiscoverJID
     * canChangeNickName
     * canOccupantsInvite
     * canOccupantsChangeSubject
     *
     * @param subdomain subdomain of chat server
     * @param roomName  room name 
     * 
     * @return a map containing these attributes.
     * 
     * @throws OpenfireClientException
     */
    public Map<String,String> getAttributes(String subdomain, String roomName) 
    throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = subdomain;
        args[1] = roomName;
        try {
            Map retval = execute("getAttributes", args); 
            return (Map<String,String>) retval.get("attributes");
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    /**
     * Get the stored extension for the chat room.
     * 
     * @param subdomain
     * @param roomName
     * 
     * @return a string with the extension stored for the chat room
     */
    public String getConferenceExtension(String subdomain, String roomName) 
    throws OpenfireClientException {
        Object[] args = new Object[2];
        args[0] = subdomain;
        args[1] = roomName;
        try {
            Map retval = execute("getConferenceExtension", args);
            return (String) retval.get(XmlRpcProvider.CONFERENCE_BRIDGE_EXTENSION);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }

    public Map<String,String> setAttributes(String subdomain, String roomName, Map newAttributes)
    throws OpenfireClientException {
        Object[] args = new Object[3];
        args[0] = subdomain;
        args[1] = roomName;
        args[2] = newAttributes;
        try {
            return ( Map <String,String> ) execute("setAttributes", args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
        
    }
    
  
  
    
    

    
    
}
