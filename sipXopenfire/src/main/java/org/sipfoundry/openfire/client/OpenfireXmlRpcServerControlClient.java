package org.sipfoundry.openfire.client;

import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.sipfoundry.openfire.plugin.presence.XmlRpcServerControlProvider;

public class OpenfireXmlRpcServerControlClient extends OpenfireXmlRpcClient {
    
    public OpenfireXmlRpcServerControlClient(String serverAddress, int port) throws Exception {
        super (XmlRpcServerControlProvider.SERVER,XmlRpcServerControlProvider.SERVICE,serverAddress,port);
    }
    
    public void enableAudit(String flag) throws OpenfireClientException {
        try {
           Object[] args = new Object[1];
           args[0] = flag;
           super.execute("enableAudit", args);
        } catch (XmlRpcException ex) {
            throw new OpenfireClientException(ex);
        }
    }
    
    public void setAuditDirectory(String directory) throws OpenfireClientException {
        try {
            Object[] args = new Object[1];
            args[0] = directory;
            super.execute("setAuditDirectory", args);
         } catch (XmlRpcException ex) {
             throw new OpenfireClientException(ex);
         } 
    }

}
