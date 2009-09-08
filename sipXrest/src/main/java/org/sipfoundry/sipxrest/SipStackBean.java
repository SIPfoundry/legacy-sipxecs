package org.sipfoundry.sipxrest;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;

import java.util.Collection;
import java.util.HashSet;

import javax.sip.SipListener;

import org.apache.log4j.Appender;
import org.sipfoundry.commons.jainsip.AbstractSipStackBean;
import org.sipfoundry.commons.jainsip.AccountManagerImpl;
import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class SipStackBean extends AbstractSipStackBean {
    public Collection<ListeningPointAddress> listeningPoints = 
        new HashSet<ListeningPointAddress>();

    @Override
    public SecureAccountManager getHashedPasswordAccountManager() {
       return RestServer.getAccountManager();
    }

    @Override
    public Collection<ListeningPointAddress> getListeningPointAddresses() {
        return listeningPoints;
    }

    @Override
    public String getLogLevel() {
       return RestServer.getRestServerConfig().getLogLevel();
    }

    @Override
    public AccountManager getPlainTextPasswordAccountManager() {
        /*
         * We dont support plain text passwords.
         */
        return null;
    }

    @Override
    public SipListener getSipListener(AbstractSipStackBean abstactSipStackBean) {
       return new SipListenerImpl();
    }

    @Override
    public Appender getStackAppender() {
       return RestServer.getAppender();
    }

    @Override
    public String getStackName() {
        return "sipxrest";
    }

}
