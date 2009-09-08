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
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"url-prefix"), "setUrlPrefix",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE, "service-description"),
                "setServiceDescription",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"sip-user-name"), "setSipUserName",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"sip-password"), "setSipPassword",0);
        digester.addCallMethod(String.format("%s/%s",REST_SERVICE,"sip-listener-class"), "setSipListenerClassName",0);
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
