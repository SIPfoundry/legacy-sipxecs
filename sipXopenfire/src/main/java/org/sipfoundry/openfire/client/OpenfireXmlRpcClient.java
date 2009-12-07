package org.sipfoundry.openfire.client;

import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;
import org.sipfoundry.openfire.plugin.presence.XmlRpcProvider;


public abstract class OpenfireXmlRpcClient {
    private static Logger logger = Logger.getLogger (OpenfireXmlRpcClient.class);
    private boolean isSecure;
   
    private String server;

    private XmlRpcClient client = new XmlRpcClient();
    
    /**
     * 
     * This method must be called before SSL connection is initialized.
     * @throws OpenfireClientException 
     */
    static {
         try {
            // Create empty HostnameVerifier
            HostnameVerifier hv = new HostnameVerifier() {
                public boolean verify(String arg0, SSLSession arg1) {
                    return true;
                }
            };

            HttpsURLConnection.setDefaultHostnameVerifier(hv);

        } catch (Exception ex) {
            logger.fatal("Unexpected exception initializing HTTPS client", ex);
            throw new RuntimeException(ex);
        }
    }

    protected OpenfireXmlRpcClient()
    {
    }
    
    protected OpenfireXmlRpcClient(String server, String service, String serverAddress, int port  ) throws Exception {
        XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        config.setEnabledForExceptions(true);
        config.setEnabledForExtensions(true);
        String url = (isSecure ? "https" : "http") + "://" + serverAddress + ":" + port + service;
        config.setServerURL(new URL(url));     
        this.client.setConfig(config);
        this.server = server;
    }
    
    
    
    protected Map execute(String method, Object[] args) throws XmlRpcException {
        
        Map retval = (Map) client.execute(server + "." + method,args);
        if ( retval.get(XmlRpcProvider.STATUS_CODE).equals(XmlRpcProvider.ERROR)) {
            throw new XmlRpcException("Processing Error " + retval.get(XmlRpcProvider.ERROR_INFO));
        }
        return retval;
        
    }

    protected Boolean executeBoolean(String method, Object[] args) throws XmlRpcException {
        
        return (Boolean) client.execute(server + "." + method,args);
    }

    protected String executeString(String method, Object[] args) throws XmlRpcException {
        
        return (String) client.execute(server + "." + method,args);
    }

    protected List<String> executeStringList(String method, Object[] args) throws XmlRpcException {
        List<String> value = new ArrayList<String>();

        Object[] result = (Object[]) client.execute(server + "." + method,args);

    	// Populate the array of strings
    	for (int i = 0;  i < result.length;  i++) {
        	value.add((String)result[i]);
        }

        return value;
    }

    protected Object[] executeObjectArray(String method, Object[] args) throws XmlRpcException {

    	return (Object[]) client.execute(server + "." + method,args);
    }

}
