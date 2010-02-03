package org.sipfoundry.sipxrest;

import gov.nist.javax.sip.ClientTransactionExt;
import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.ServerTransactionExt;
import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.message.MessageExt;

import java.util.Collection;
import java.util.HashSet;
import java.util.Properties;

import javax.sip.ListeningPoint;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TransactionAlreadyExistsException;
import javax.sip.TransactionUnavailableException;
import javax.sip.header.CallIdHeader;
import javax.sip.header.ViaHeader;
import javax.sip.message.Request;

import org.apache.log4j.Appender;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.jainsip.AbstractSipStackBean;
import org.sipfoundry.commons.jainsip.ListeningPointAddress;

public class SipStackBean extends AbstractSipStackBean {
    private Collection<ListeningPointAddress> listeningPoints = new HashSet<ListeningPointAddress>();
    public static Logger logger = Logger.getLogger(SipStackBean.class);
    public static final String COLON = ":";

    SipListenerImpl sipListener;

    public ListeningPointExt getListeningPoint(String transport) {
        SipProvider sipProvider = getSipProvider(transport);
        return (ListeningPointExt) sipProvider.getListeningPoint(transport);
    }

    public SipProvider getSipProvider(String transport) {
        for (ListeningPointAddress lpa : this.listeningPoints) {
            if (lpa.getTransport().equalsIgnoreCase(transport))
                return lpa.getSipProvider();
        }
        return null;
    }

    public SipStackBean() throws Exception {
        super();
        this.listeningPoints.add(new ListeningPointAddressImpl("tcp"));
        this.listeningPoints.add(new ListeningPointAddressImpl("udp"));
        super.init();
    }

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
        if (sipListener == null) {
            sipListener = new SipListenerImpl();
        }
        return sipListener;
    }

    @Override
    public Appender getStackAppender() {
        return RestServer.getAppender();
    }

    @Override
    public String getStackName() {
        return "sipxrest";
    }

    @Override
    public Properties getExtraStackProperties() {
       Properties properties = new Properties();
       properties.setProperty("gov.nist.javax.sip.IS_BACK_TO_BACK_USER_AGENT", "true");
       properties.setProperty("gov.nist.javax.sip.MAX_FORK_TIME_SECONDS", "180");
       return properties;
    } 

}
