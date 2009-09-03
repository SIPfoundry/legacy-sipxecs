package org.sipfoundry.commons.callcontrollerconfig;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class CallControllerConfigFileParser {
	
	public static final String CALL_CONTROLLER_CONFIG = "call-controller-config";
	
	  /**
     * Add the digester rules.
     *
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate(CALL_CONTROLLER_CONFIG, CallControllerConfig.class);
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"ip-address"), "setIpAddress",0);
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"sip-port"), "setSipPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"https-port"), "setHttpPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"external-http-port"), "setPublicHttpPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"sipx-proxy-domain"), "setSipxProxyDomain",0);
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"log-directory"), "setLogDirectory",0);
        digester.addCallMethod( String.format("%s/%s", CALL_CONTROLLER_CONFIG,"log-level"), "setLogLevel",0);
    }
    
    public CallControllerConfig createCallControllerConfig(String url) {
  	  Digester digester = new Digester();
        addRules(digester);
        try {
            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            CallControllerConfig callControllerConfig = (CallControllerConfig) digester.getRoot();
            return callControllerConfig;
        } catch (Exception ex) {
      	  throw new RuntimeException(ex);
        }
     
  }

}
