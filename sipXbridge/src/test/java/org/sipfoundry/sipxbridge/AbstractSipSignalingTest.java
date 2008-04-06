/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import javax.sip.ListeningPoint;
import javax.sip.SipListener;
import javax.sip.SipProvider;

import org.sipfoundry.sipxbridge.*;

import gov.nist.javax.sip.clientauthutils.*;

import junit.framework.TestCase;

public abstract class AbstractSipSignalingTest extends TestCase {
    protected AccountManagerImpl accountManager = null;
    protected SipProvider provider;
   
    @Override
    public void setUp() throws Exception {
        Gateway.start();
        accountManager = Gateway.getAccountManager();
         provider = Gateway.getWanProvider();
        provider.addSipListener(getSipListener());

    }

    @Override
    public void tearDown() throws Exception {
        provider.removeSipListener(this.getSipListener());
        Gateway.stop();

    }

    public abstract SipListener getSipListener();

}
