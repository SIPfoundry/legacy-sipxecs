package org.sipfoundry.openfire.client;

import java.util.Map;

import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.openfire.plugin.presence.XmlRpcChatRoomManagementProvider;

public class OpenfireXmlRpcChatRoomManagementClient extends OpenfireXmlRpcClient {
    
    
    public OpenfireXmlRpcChatRoomManagementClient(String serverAddress, int port) throws Exception {
       super(XmlRpcChatRoomManagementProvider.SERVER, XmlRpcChatRoomManagementProvider.SERVICE_PATH, serverAddress,port);
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
    
    
    
}
