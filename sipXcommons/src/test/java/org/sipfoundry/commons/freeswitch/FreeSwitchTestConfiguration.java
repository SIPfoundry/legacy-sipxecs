/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
/**
 *
 */
package org.sipfoundry.commons.freeswitch;

import org.apache.log4j.Logger;

/**
 * @author mardy
 *
 */
public class FreeSwitchTestConfiguration implements FreeSwitchConfigurationInterface {

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getDocDirectory()
     */
    @Override
    public String getDocDirectory() {
        return null;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getEventSocketPort()
     */
    @Override
    public int getEventSocketPort() {
        return 0;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getLogFile()
     */
    @Override
    public String getLogFile() {
        return null;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getLogLevel()
     */
    @Override
    public String getLogLevel() {
        return null;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getLogger()
     */
    @Override
    public Logger getLogger() {
        return Logger.getLogger("org.sipfoundry.commons");
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getRealm()
     */
    @Override
    public String getRealm() {
        return null;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#getSipxchangeDomainName()
     */
    @Override
    public String getSipxchangeDomainName() {
        return null;
    }

    /* (non-Javadoc)
     * @see org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface#setRealm(java.lang.String)
     */
    @Override
    public void setRealm(String realm) {
    }

}
