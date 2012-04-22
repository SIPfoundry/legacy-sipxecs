package org.sipfoundry.sipxprovision.auto;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLSession;

import org.apache.log4j.Logger;


public class CertificateHostnameVerifier  implements HostnameVerifier {

    private static final Logger LOG = Logger.getLogger("CertificateHostnameVerifier");
    private String m_domainName;

    public CertificateHostnameVerifier(String domainName) {
        m_domainName = domainName;
    }
    @Override
    public boolean verify(String hostname, SSLSession session) {
        LOG.debug("Hostname to verify: "+hostname);
        LOG.debug("Trusted domain: "+m_domainName);
        return hostname != null && m_domainName != null && hostname.contains(m_domainName);
    }

}
