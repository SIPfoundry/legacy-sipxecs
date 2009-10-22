/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class SipxSaaDaoListenerTest extends TestCase {

    public void testOnSaveUser() {
        SipxSaaDaoListener sipxSaaDaoListener = new SipxSaaDaoListener();

        SipxReplicationContext sipxReplicationContext = EasyMock.createMock(SipxReplicationContext.class);
        sipxSaaDaoListener.setSipxReplicationContext(sipxReplicationContext);

        ConfigurationFile configurationFile = EasyMock.createMock(ConfigurationFile.class);
        sipxSaaDaoListener.setConfigurationFile(configurationFile);

        sipxReplicationContext.replicate(configurationFile);
        EasyMock.expectLastCall();
        EasyMock.replay(sipxReplicationContext);

        sipxSaaDaoListener.onSave(new User());
        EasyMock.verify(sipxReplicationContext);
    }

    public void testOnDeleteUser() {
        SipxSaaDaoListener sipxSaaDaoListener = new SipxSaaDaoListener();

        SipxReplicationContext sipxReplicationContext = EasyMock.createMock(SipxReplicationContext.class);
        sipxSaaDaoListener.setSipxReplicationContext(sipxReplicationContext);

        ConfigurationFile configurationFile = EasyMock.createMock(ConfigurationFile.class);
        sipxSaaDaoListener.setConfigurationFile(configurationFile);

        sipxReplicationContext.replicate(configurationFile);
        EasyMock.expectLastCall();
        EasyMock.replay(sipxReplicationContext);

        sipxSaaDaoListener.onDelete(new User());
        EasyMock.verify(sipxReplicationContext);
    }

    public void testOnSaveNonUser() {
        SipxSaaDaoListener sipxSaaDaoListener = new SipxSaaDaoListener();
        sipxSaaDaoListener.onSave(new SipxSaaService());
    }

    public void testOnDeleteNonUser() {
        SipxSaaDaoListener sipxSaaDaoListener = new SipxSaaDaoListener();
        sipxSaaDaoListener.onDelete(new SipxSaaService());
    }
}
