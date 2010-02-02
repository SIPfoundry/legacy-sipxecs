package org.sipfoundry.siptester;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;

import java.util.Collection;
import java.util.HashSet;
import java.util.Properties;

import javax.sip.SipListener;

import org.apache.log4j.Appender;
import org.sipfoundry.commons.jainsip.AbstractSipStackBean;
import org.sipfoundry.commons.jainsip.ListeningPointAddress;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

public class SipStackBean extends AbstractSipStackBean {
    private static int count;
    private static Appender appender;
    static {
        try {
            appender = new SipFoundryAppender(new SipFoundryLayout(), "sipxtester.log");
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    HashSet<ListeningPointAddress> listeningPointAddresses = new HashSet<ListeningPointAddress>();

    private EmulatedEndpoint endpoint;

    protected SipListenerImpl sipListener;

    public SipStackBean(EmulatedEndpoint endpoint) {
        super();
        this.endpoint = endpoint;
    }

    @Override
    public Properties getExtraStackProperties() {
        Properties properties = new Properties();
        properties.setProperty("gov.nist.javax.sip.DELIVER_UNSOLICITED_NOTIFY", "true");
        properties.setProperty("gov.nist.javax.sip.MIN_KEEPALIVE_TIME_SECONDS", "1");
        if ( this.endpoint.getTraceEndpoint().getBehavior() == Behavior.PROXY ) {
            properties.setProperty("javax.sip.AUTOMATIC_DIALOG_SUPPORT", "off");
            
        }
        properties.setProperty("gov.nist.javax.sip.THREAD_POOL_SIZE", "32");
        return properties;
    }

    @Override
    public SecureAccountManager getHashedPasswordAccountManager() {
        try {
            return new AccountManagerImpl();
        } catch (Exception ex) {
            throw new SipTesterException(ex);
        }
    }

    @Override
    public Collection<ListeningPointAddress> getListeningPointAddresses() {
        return this.listeningPointAddresses;
    }

    @Override
    public String getLogLevel() {
        return SipTester.getTesterConfig().getLogLevel();
    }

    @Override
    public AccountManager getPlainTextPasswordAccountManager() {
        return null;
    }

    @Override
    public SipListener getSipListener(AbstractSipStackBean abstactSipStackBean) {
        if (this.sipListener == null) {
            this.sipListener = new SipListenerImpl(endpoint);
        }
        return this.sipListener;
    }

    @Override
    public Appender getStackAppender() {
        try {
            return appender;
        } catch (Exception ex) {
            System.out.println("Bad appender");
            return null;
        }
    }

    @Override
    public String getStackName() {
        return "tester" + count++;
    }

    public void addListeningPoint(ListeningPointAddressImpl lpa) {
        this.listeningPointAddresses.add(lpa);
    }

    public SipListenerImpl getSipListener() {
        return this.sipListener;
    }

}
