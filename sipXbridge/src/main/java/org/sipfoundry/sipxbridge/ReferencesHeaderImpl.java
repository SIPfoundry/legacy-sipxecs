package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.header.SIPHeader;

import java.text.ParseException;

import javax.sip.header.ExtensionHeader;



public class ReferencesHeaderImpl  implements ReferencesHeader,ExtensionHeader {
    
    private static final long serialVersionUID = -6235038166218875753L;
    private String callId ;    
    private String rel;
    private String headerName = ReferencesHeader.NAME;

    public ReferencesHeaderImpl () {
        this.rel = ReferencesHeader.CHAIN;
    }
    
    public String getCallId() {
        return callId;
    }
    
    public void setCallId(String callId) {
        this.callId = callId;
    }

    public String getRel() {
       return rel;
    }
    
    public void setRel(String rel) {
        this.rel = rel;
    }

    public String getValue() {
       return callId + ";rel="+rel;
    }

    public void setValue(String value) throws ParseException {
        throw new UnsupportedOperationException("Op not supported");
    }

    public String getName() {
        return NAME;
    }
    
    public Object clone() {
        ReferencesHeaderImpl references = new ReferencesHeaderImpl();
        references.callId = this.callId;
        references.rel = this.rel;
        return references;
        
    }
    
    public String toString() {
        return new StringBuffer().append(NAME).append(":").append(getValue()).toString();
    }

   

}
