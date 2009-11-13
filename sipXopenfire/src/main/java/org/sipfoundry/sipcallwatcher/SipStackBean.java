package org.sipfoundry.sipcallwatcher;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

import java.util.Collection;
import java.util.HashSet;
import java.util.Properties;

import javax.sip.ClientTransaction;
import javax.sip.SipListener;

import org.apache.log4j.Appender;
import org.sipfoundry.commons.jainsip.AbstractSipStackBean;
import org.sipfoundry.commons.jainsip.ListeningPointAddress;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;

public class SipStackBean extends AbstractSipStackBean implements AccountManager {
    
    Collection<ListeningPointAddress> lpaSet = new HashSet<ListeningPointAddress>();
    private Subscriber subscriber;
    private ListeningPointAddressImpl tcpListeningPointAddress;
    private ListeningPointAddressImpl udpListeningPointAddress;
    
    public SipStackBean() {
        super();
        
    }
    
    public Subscriber getSubscriber() {      
        this.subscriber.setProvider(udpListeningPointAddress.getSipProvider());
        return subscriber;
        
    }
    
   
    @Override
    public Collection<ListeningPointAddress> getListeningPointAddresses() {   
        this.tcpListeningPointAddress = new ListeningPointAddressImpl("tcp");
        this.udpListeningPointAddress = new ListeningPointAddressImpl("udp");
        lpaSet.add(this.tcpListeningPointAddress);
        lpaSet.add(this.udpListeningPointAddress);
        return lpaSet;
    }

    @Override
    public String getLogLevel() {
        return CallWatcher.getConfig().getLogLevel();
    }

    @Override
    public SipListener getSipListener(AbstractSipStackBean abstactSipStackBean) {
         if ( this.subscriber != null ) return this.subscriber ;
         else {
             this.subscriber = new Subscriber(this);
          
             return this.subscriber;
         }
    }
    
    

    @Override
    public Appender getStackAppender() {
        return SipXOpenfirePlugin.getLogAppender();
    }

    @Override
    public String getStackName() {
        return "sipxcallwatcher";
    }

    @Override
    public AccountManager getPlainTextPasswordAccountManager() {      
        return this;
    }

    /**
     * Special users are not in the hashed password database.
     */
    @Override
    public SecureAccountManager getHashedPasswordAccountManager() {
        return null;
    }

    @Override
    public UserCredentials getCredentials(ClientTransaction ctx, String authRealm) {
        return new UserCredentials() {
            public String getPassword() {
                return CallWatcher.getConfig().getPassword();
            }

            public String getSipDomain() {
                return CallWatcher.getConfig().getProxyDomain();
            }

            public String getUserName() {
                return CallWatcher.getConfig().getUserName();
            }
        };
    }

    @Override
    public Properties getExtraStackProperties() {
        /*
         * Properties extraProperties = new Properties();
         * extraProperties.setProperty("gov.nist.javax.sip.DELIVER_TERMINATED_EVENT_FOR_NULL_DIALOG",
         * "true"); return extraProperties;
         */
        return null;
    }

}
