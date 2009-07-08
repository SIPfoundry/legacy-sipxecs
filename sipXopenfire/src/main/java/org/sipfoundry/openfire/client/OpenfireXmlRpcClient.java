package org.sipfoundry.openfire.client;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

import org.apache.log4j.Logger;

public abstract class OpenfireXmlRpcClient {
    private static Logger logger = Logger.getLogger (OpenfireXmlRpcClient.class);
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
}
