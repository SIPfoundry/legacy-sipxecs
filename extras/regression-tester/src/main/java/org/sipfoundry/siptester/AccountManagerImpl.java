package org.sipfoundry.siptester;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;
import javax.sip.header.FromHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.jainsip.AbstractAccountManager;



public class AccountManagerImpl extends AbstractAccountManager {
    
    
    private static final Logger logger = Logger.getLogger(AccountManagerImpl.class);
    HashMap<String, UserCredentialHash> plainTextCredentials = new HashMap<String, UserCredentialHash>();
    
    /**
     * to hex converter
     */
    private static final char[] toHex = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * Converts b[] to hex string.
     * 
     * @param b the bte array to convert
     * @return a Hex representation of b.
     */
    public static String toHexString(byte b[]) {
        int pos = 0;
        char[] c = new char[b.length * 2];
        for (int i = 0; i < b.length; i++) {
            c[pos++] = toHex[(b[i] >> 4) & 0x0F];
            c[pos++] = toHex[b[i] & 0x0f];
        }
        return new String(c);
    }

    /**
     * Defined in rfc 2617 as H(data) = MD5(data);
     * 
     * @param data data
     * @return MD5(data)
     */
    public static String H(String data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");

            return toHexString(digest.digest(data.getBytes()));
        } catch (NoSuchAlgorithmException ex) {
            // shouldn't happen
            throw new RuntimeException("Failed to instantiate an MD5 algorithm", ex);
        }
    }

    private static String computePasswordHash(String userName, String passwd) {
        if (userName == null || passwd == null)
            throw new NullPointerException(
                    "Null parameter to MessageDigestAlgorithm.calculateResponse()");
        String realmValue = SipTester.getTesterConfig().getSipxProxyDomain();
        String A1 = userName + ":" + realmValue + ":" + passwd;
        return H(A1);

    }

    public AccountManagerImpl() throws Exception {
        super();
    }

    /**
     * Add a plain text password account to this account manager. Clear text sip passwords
     * are made available for users with special IDs such as ~~id~callWatcher. These
     * are not part of the validuser.xml database and must be provided to the service
     * by other means ( configuration files ).
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
        UserCredentialHash credHash = null;
        try {
            logger.debug("getCredentialHash " + realm);
            logger.debug("request " + clientTransaction.getRequest());
            credHash = super.getCredentialHash(clientTransaction, realm);
            if (credHash == null) {
                Request request = clientTransaction.getRequest();
                FromHeader from = (FromHeader) request.getHeader(FromHeader.NAME);
                String fromUser = ((SipURI) from.getAddress().getURI()).getUser();
                credHash = this.plainTextCredentials.get(fromUser);
            }
            return credHash;
        } finally {
            logger.debug("getCredentialHash returning " + credHash);
            
        }

    }
}
