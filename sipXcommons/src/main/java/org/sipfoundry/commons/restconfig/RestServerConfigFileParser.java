/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.restconfig;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class RestServerConfigFileParser {
	
	public static final String REST_CONFIG = "rest-config";
	
	  /**
     * Add the digester rules.
     *
     * @param digester
     */
    private static void addRules(Digester digester) {
        digester.addObjectCreate(REST_CONFIG, RestServerConfig.class);
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"ip-address"), "setIpAddress",0);
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"sip-port"), "setSipPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"https-port"), "setHttpPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"external-http-port"), "setPublicHttpPort",0, new Class[] {
            Integer.class});
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"sipx-proxy-domain"), "setSipxProxyDomain",0);
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"log-directory"), "setLogDirectory",0);
        digester.addCallMethod( String.format("%s/%s", REST_CONFIG,"log-level"), "setLogLevel",0);
    }
    
    public RestServerConfig parse(String url) {
        Digester digester = new Digester();
        addRules(digester);
        try {
            InputSource inputSource = new InputSource(url);
            digester.parse(inputSource);
            RestServerConfig restServerConfig = (RestServerConfig) digester.getRoot();
            return restServerConfig;
        } catch (Exception ex) {
      	  throw new RuntimeException(ex);
        }
     
  }

}
