/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrest;

import java.io.InputStream;

import org.apache.commons.digester.Digester;
import org.xml.sax.InputSource;

public class MetaInfParser {
    private static final String REST_SERVICE = "rest-service";
    private static void addRules(Digester digester) {
        digester.addObjectCreate(REST_SERVICE, MetaInf.class);
        digester.addCallMethod(String.format("%s/%s", REST_SERVICE,"plugin-class"), 
                "setPluginClass",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"security-level"), "setSecurity",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"uri-prefix"), "setUriPrefix",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE, "service-description"),
                "setServiceDescription",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"sip-convergence-name"), "setSipConvergenceName",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"sip-listener-class"), "setSipListenerClassName",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"remote-authentication-method"), 
                    "setRemoteAuthenticationMethod",0);
        
    }
    
    private Digester digester;
    
    public MetaInfParser() {
        this.digester = new Digester();
        addRules(digester);
    
    }
    MetaInf parse (InputStream inputStream) {
         try {
            InputSource inputSource = new InputSource(inputStream);
            digester.parse(inputSource);
            MetaInf metaInf = (MetaInf) digester.getRoot();
            return metaInf;
        } catch (Exception ex) {
          throw new RuntimeException(ex);
        }
     
  }

}
