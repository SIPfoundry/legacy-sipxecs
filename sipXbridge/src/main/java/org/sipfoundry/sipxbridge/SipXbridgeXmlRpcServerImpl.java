package org.sipfoundry.sipxbridge;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.xmlrpc.SipXbridgeXmlRpcServer;

public class SipXbridgeXmlRpcServerImpl implements SipXbridgeXmlRpcServer {

    private static Logger logger = Logger.getLogger(SipXbridgeXmlRpcServerImpl.class);
    
    private String formatStackTrace(Throwable ex) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        ex.printStackTrace(pw);
        return sw.getBuffer().toString();
    }
    
    private HashMap<String, Object> createSuccessMap() {
        HashMap<String, Object> retval = new HashMap<String, Object>();
        retval.put(STATUS_CODE, OK);
        return retval;
    }

    

    
    public Map<String, Object> getRegistrationStatus() {
        HashMap<String, Object> retval = createSuccessMap();
        try {
            HashSet<Map<String,String>> registrationRecords = new HashSet<Map<String,String>>();
            for (ItspAccountInfo itspAccount : Gateway.getAccountManager().getItspAccounts()) {
                if ( itspAccount.isRegisterOnInitialization() ) {
                    registrationRecords.add(itspAccount.getRegistrationRecord().getMap());
                }
            }
            retval.put(STATUS_CODE, OK);
            retval.put(REGISTRATION_RECORDS, registrationRecords.toArray());

        } catch (Throwable ex) {        
            retval.put(STATUS_CODE, ERROR);
            retval.put(ERROR_INFO, formatStackTrace(ex));
        }
        
        return retval;
    }

    public Map<String,Object> getCallCount() {
        HashMap<String, Object> retval = createSuccessMap();
        try {
           
            retval.put(STATUS_CODE, OK);
            retval.put(CALL_COUNT, new Integer(Gateway.getCallCount())).toString();

        } catch (Throwable ex) {        
            retval.put(STATUS_CODE, ERROR);
            retval.put(ERROR_INFO, formatStackTrace(ex));
        }
        
        return retval;
    }
 

 
    public Map<String, Object> start() {
       HashMap<String, Object> retval = createSuccessMap();
      
       logger.debug("Gateway.start()");
       try {
           Gateway.start();
       } catch (Throwable ex) {
           retval.put(STATUS_CODE, ERROR);
           retval.put(ERROR_INFO, formatStackTrace(ex));
       } 
      
       return retval;
    }

 
    public Map<String, Object> stop() {
        
        logger.debug("Gateway.stop()");
        HashMap<String, Object> retval = createSuccessMap();

        try {
            Gateway.stop();
        } catch (Throwable ex) {
            retval.put(STATUS_CODE, ERROR);
            retval.put(ERROR_INFO, formatStackTrace(ex));
        }
        return retval;
    }
    
    

}
