package org.sipfoundry.openfire.plugin.presence;

import java.io.File;
import java.util.Map;

import org.apache.log4j.Logger;
import org.jivesoftware.util.JiveGlobals;
import org.xmpp.component.Log;

public class XmlRpcServerControlProvider extends XmlRpcProvider {
    static SipXOpenfirePlugin plugin;

    public static final String SERVICE = "serverctl";

    public static final String SERVER = "serverControl";

    private Logger log = Logger.getLogger(XmlRpcServerControlProvider.class);

    
    public Map<String,Object> setDomain(String domain) {
        try {
            log.info("setDomain " + domain );
            JiveGlobals.setProperty("xmpp.domain",domain);
            return createSuccessMap();
        } catch ( Exception ex) {
            return super.createErrorMap(ErrorCode.SERVER_CONTROL_FAILED, ex.getMessage());
        }
    }
    
    public Map<String,Object> enableAudit(String flag) {
        try {
            log.info("enableAudit " + flag);
            JiveGlobals.setProperty("xmpp.audit.active",flag);
            return createSuccessMap();
        } catch (Exception ex) {
            return super.createErrorMap(ErrorCode.SERVER_CONTROL_FAILED, ex.getMessage());
        }
    }
    
    public Map<String,Object> setAuditDirectory(String directory) {
        if ( ! new File(directory).isDirectory() ) {
            return createErrorMap(
                    ErrorCode.INVALID_ARGUMENT,"Not a directory " + directory);
        }
        try {
            log.info("setAuditDirectory" + directory);          
            JiveGlobals.setProperty("xmpp.audit.logdir", directory);
            return  createSuccessMap();
        } catch (Exception ex) {
            return super.createErrorMap(ErrorCode.SERVER_CONTROL_FAILED, ex.getMessage());
        }
    }
}
