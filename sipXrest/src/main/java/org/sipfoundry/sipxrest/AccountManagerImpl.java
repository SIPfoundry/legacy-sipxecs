package org.sipfoundry.sipxrest;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import java.security.MessageDigest;
import java.util.HashMap;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;
import javax.sip.header.FromHeader;
import javax.sip.message.Request;

import org.sipfoundry.commons.jainsip.AbstractAccountManager;

public class AccountManagerImpl extends AbstractAccountManager {
    HashMap<String, UserCredentialHash> plainTextCredentials = new HashMap<String, UserCredentialHash>();

    private static String computePasswordHash(String userName, String passwd) {
        if (userName == null || passwd == null)
            throw new NullPointerException(
                    "Null parameter to MessageDigestAlgorithm.calculateResponse()");
        String realmValue = RestServer.getRestServerConfig().getSipxProxyDomain();
        String A1 = userName + ":" + realmValue + ":" + passwd;
        return Util.H(A1);

    }

    public AccountManagerImpl() throws Exception {
        super();
    }

    /**
     * Add a plain text password account to this account amanger.
     * 
     * @param userName
     * @param realm
     * @param password
     */
    public void addAccount(String userName, String password) {
        UserCredentialHash credHash = new UserCredentialsImpl(userName, computePasswordHash(
                userName, password));
        this.plainTextCredentials.put(userName, credHash);

    }

    @Override
    public UserCredentialHash getCredentialHash(ClientTransaction clientTransaction, String realm) {
        UserCredentialHash credHash = super.getCredentialHash(clientTransaction, realm);
        if (credHash == null) {
            Request request = clientTransaction.getRequest();
            FromHeader from = (FromHeader) request.getHeader(FromHeader.NAME);
            String fromUser = ((SipURI) from.getAddress().getURI()).getUser();
            return this.plainTextCredentials.get(fromUser);
        } else {
            return credHash;
        }

    }

}
