package org.sipfoundry.sipxbridge;

import javax.sip.header.ExtensionHeader;
import javax.sip.header.Header;

public interface ReferencesHeader  extends Header {
 public static final String NAME = "References";
    
    public static final String CHAIN = "chain";
    
    public static final String INQUIRY =  "inquiry";
    
    public static final String REFER = "refer" ;
    
    public static final String SEQUEL = "sequel";
    
    public static final String XFER =  "xfer";
    
    public static final String X_SIPX_BRANCH = "x-sipx-branch";
    
    public String getCallId();
    
    public void setCallId(String callId);
    
    public String getRel();
    
    public void setRel(String rel);
    
    public String getBranch();
    
    public void setBranch(String branch);

}
