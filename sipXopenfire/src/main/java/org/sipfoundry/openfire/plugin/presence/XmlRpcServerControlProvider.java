package org.sipfoundry.openfire.plugin.presence;

import java.util.Map;

import org.apache.log4j.Logger;
import org.xmpp.component.Log;

public class XmlRpcServerControlProvider extends XmlRpcProvider {
    static SipXOpenfirePlugin plugin;

    public static final String SERVICE = "serverctl";

    public static final String SERVER = "serverControl";

    private Logger log = Logger.getLogger(XmlRpcServerControlProvider.class);

    public Map<String, Object> stop() {
        try {
            log.info("Stopping");
            plugin.stopServer();
            return createSuccessMap();
        } catch (Exception ex) {
            return super.createErrorMap(ErrorCode.SERVER_STOP_FAILED, ex.getMessage());
        }
    }
    
    public Map<String, Object> start() {
        try {
            log.info("Start");
            plugin.startServer();
            return createSuccessMap();
        } catch (Exception ex) {
            return super.createErrorMap(ErrorCode.SERVER_STOP_FAILED, ex.getMessage());
        }
    }

}
