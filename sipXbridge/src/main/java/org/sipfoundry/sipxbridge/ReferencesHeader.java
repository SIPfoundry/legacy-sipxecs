package org.sipfoundry.sipxbridge;

import javax.sip.header.ExtensionHeader;

public interface ReferencesHeader extends ExtensionHeader {
 public static final String NAME = "References";
    
    public static final String CHAIN = "chain";
    
    public static final String INQUIRY =  "inquiry";
    
    public static final String REFER = "refer" ;
    
    public static final String SEQUEL = "sequel";
    
    public static final String XFER =  "xfer";
    
    public String getCallId();
    
    public void setCallId(String callId);
    
    public String getRel();
    
    public void setRel(String rel);

}
