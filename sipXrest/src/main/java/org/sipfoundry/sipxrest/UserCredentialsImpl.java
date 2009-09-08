package org.sipfoundry.sipxrest;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

public class UserCredentialsImpl implements UserCredentialHash {
    public String credHash;
    private String userName;
    
    public UserCredentialsImpl( String userName, String credHash) {
        this.credHash = credHash;
        this.userName = userName;
    }

    @Override
    public String getHashUserDomainPassword() {
        return credHash;
    }

    @Override
    public String getSipDomain() {
       return RestServer.getRestServerConfig().getSipxProxyDomain();
    }

    @Override
    public String getUserName() {
        return userName;
    }

}
