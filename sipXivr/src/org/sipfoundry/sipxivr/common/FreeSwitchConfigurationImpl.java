/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr.common;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

public class FreeSwitchConfigurationImpl implements FreeSwitchConfigurationInterface {
    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String m_logFile;
    private int m_eventSocketPort;
    private String m_docDirectory;
    private String m_sipxChangeDomainName;
    private String m_realm;

    @Override
    public Logger getLogger() {
        return LOG;
    }

    @Override
    public String getLogLevel() {
        return SipFoundryLayout.getSipFoundryLogLevel().toString();
    }

    public void setLogFile(String logFile) {
        m_logFile = logFile;
    }

    @Override
    public String getLogFile() {
        return m_logFile;
    }

    public void setEventSocketPort(int port) {
        m_eventSocketPort = port;
    }

    @Override
    public int getEventSocketPort() {
        return m_eventSocketPort;
    }

    public void setDocDirectory(String docDirectory) {
        m_docDirectory = docDirectory;
    }

    @Override
    public String getDocDirectory() {
        return m_docDirectory;
    }

    public void setSipxchangeDomainName(String domain) {
        m_sipxChangeDomainName = domain;
    }

    @Override
    public String getSipxchangeDomainName() {
        return m_sipxChangeDomainName;
    }

    @Override
    public String getRealm() {
        return m_realm;
    }

    @Override
    public void setRealm(String realm) {
        m_realm = realm;
    }
}
