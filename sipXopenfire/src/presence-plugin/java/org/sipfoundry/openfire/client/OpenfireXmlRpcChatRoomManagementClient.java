/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.openfire.client;

import java.util.Map;

import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.openfire.plugin.presence.XmlRpcChatRoomManagementProvider;

public class OpenfireXmlRpcChatRoomManagementClient extends OpenfireXmlRpcClient {


    public OpenfireXmlRpcChatRoomManagementClient(String serverAddress, int port, String sharedSecret) throws Exception {
       super(XmlRpcChatRoomManagementProvider.SERVER, XmlRpcChatRoomManagementProvider.SERVICE_PATH, serverAddress,port, sharedSecret);
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
