/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.Arrays;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.setting.Group;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.reset;
import static org.easymock.EasyMock.verify;

public class SipxOpenfireDaoListenerTest extends TestCase {

    private List<ConfigurationFile> m_configFiles;
    private SipxReplicationContext m_sipxReplicationContext;
    private SipxOpenfireDaoListener m_sipxOpenfireDaoListener;
    private Group m_userGroup;
    private Group m_nonUserGroup;
    
    @Override
    protected void setUp() throws Exception {

        ConfigurationFile [] configFilesArray = {createMock(ConfigurationFile.class),
                createMock(ConfigurationFile.class)};
        m_configFiles = Arrays.asList(configFilesArray);

        m_sipxReplicationContext = createMock(SipxReplicationContext.class);
        m_sipxOpenfireDaoListener = new SipxOpenfireDaoListener();
        m_sipxOpenfireDaoListener.setConfigurationFiles(m_configFiles);
        m_sipxOpenfireDaoListener.setSipxReplicationContext(m_sipxReplicationContext);

        m_userGroup = new Group();
        m_userGroup.setResource(User.GROUP_RESOURCE_ID);

        m_nonUserGroup = new Group();
        m_nonUserGroup.setResource("Non-User-Group");
    }

    public void testOnSave() {
        onSaveTest(true, new User());
        onSaveTest(true, new Conference());
        onSaveTest(true, m_userGroup);

        onSaveTest(false, m_nonUserGroup);
        onSaveTest(false, new SipxOpenfireService());
    }

    public void testOnDelete() {
        onDeleteTest(true, new User());
        onDeleteTest(true, new Conference());
        onDeleteTest(true, m_userGroup);

        onDeleteTest(false, m_nonUserGroup);
        onDeleteTest(false, new SipxOpenfireService());
    }

    private void onSaveTest(boolean replicate, Object entity) {
        expectReplication(replicate);
        m_sipxOpenfireDaoListener.onSave(entity);
        verify(m_sipxReplicationContext);
    }

    private void onDeleteTest(boolean replicate, Object entity) {
        expectReplication(replicate);
        m_sipxOpenfireDaoListener.onDelete(entity);
        verify(m_sipxReplicationContext);
    }

    private void expectReplication(boolean replicate) {
        reset(m_sipxReplicationContext);

        if (replicate) {
            for (ConfigurationFile configFile : m_configFiles) {
                m_sipxReplicationContext.replicate(configFile);
                    expectLastCall().times(1);
            }
        }
        replay(m_sipxReplicationContext);
    }
}
