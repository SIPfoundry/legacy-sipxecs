package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.header.SIPHeader;

import java.text.ParseException;

import javax.sip.header.ExtensionHeader;



public class ReferencesHeaderImpl extends SIPHeader implements ReferencesHeader,ExtensionHeader  {
    
    private static final long serialVersionUID = -6235038166218875753L;
    private String callId ;    
    private String rel;
    private String headerName = ReferencesHeader.NAME;
    private String branch;
  

    public ReferencesHeaderImpl () {
        this.rel = ReferencesHeader.CHAIN;
    }
    
    public ReferencesHeaderImpl(String callId, String rel, String branch) {
        this.callId = callId;
        this.rel = rel;
        this.branch = branch;
    }
    
    public void setBranch(String branch) {
        this.branch  = branch;
    }
    
    public String getBranch() {
        return branch;
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
        if ( branch != null ) {
            return callId + ";rel="+rel+";x-sipx-branch="+branch;
        } else {
            return callId + ";rel="+rel;
        }
    }

    public void setValue(String value) throws ParseException {
        throw new UnsupportedOperationException("Op not supported");
    }
    
    public static ReferencesHeader createReferencesHeader(String header ) {
        String[] parts = header.split(":");
        if ( ! parts[0].equalsIgnoreCase(ReferencesHeader.NAME)) {
            throw new IllegalArgumentException("bad header name " + parts[0]);
        }
        String[] parts1 = parts[1].split(";rel=");
        String callId = parts1[0];
        String[] parts2 = parts1[1].split(";x-sipx-branch=");
        String rel = parts2[0];
        String branch = null;
        if ( parts2.length == 2) {
            branch = parts2[1];
        }
        return new ReferencesHeaderImpl(callId,rel,branch);
    }

    public String getName() {
        return NAME;
    }
    
    public Object clone() {
        ReferencesHeaderImpl references = new ReferencesHeaderImpl();
        references.callId = this.callId;
        references.rel = this.rel;
        references.branch = this.branch;
        return references;
        
    }
    

    @Override
    protected String encodeBody() {
        return getValue();
    }

   

}
