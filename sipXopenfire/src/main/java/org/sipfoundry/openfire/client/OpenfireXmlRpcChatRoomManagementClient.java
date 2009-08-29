package org.sipfoundry.openfire.client;

import java.util.Map;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.sipfoundry.openfire.plugin.presence.XmlRpcChatRoomManagementProvider;
import org.sipfoundry.openfire.plugin.presence.XmlRpcProvider;

public class OpenfireXmlRpcChatRoomManagementClient extends OpenfireXmlRpcClient {
    
    
    public OpenfireXmlRpcChatRoomManagementClient(String serverAddress, int port) throws Exception {
       super(XmlRpcChatRoomManagementProvider.SERVER, XmlRpcChatRoomManagementProvider.SERVICE, serverAddress,port);
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
    
    
   public void inviteOccupant(String subdomain, String roomName,
           String member, String password, String reason) throws OpenfireClientException {
       Object[] args = new Object[5];
       args[0] = subdomain;
       args[1] = roomName;
       args[2] = member;
       args[3] = password == null ? "":password;
       args[4] = reason;
       try {
           Map retval = execute("inviteOccupant",args);
       } catch ( XmlRpcException ex) {
           ex.printStackTrace();
           throw new OpenfireClientException(ex);
       }
   }
  

    
}
