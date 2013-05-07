/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.ivr;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContextImpl;
import org.sipfoundry.sipxconfig.commserver.imdb.ImdbTestCase;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class IvrConfigTestIntegration extends ImdbTestCase {
    private CoreContext m_coreContext;
    private ConfigManager m_configManager;
    private SipxReplicationContextImpl m_sipxReplicationContextImpl;
    private AddressManager m_addressManager;
    private PermissionManager m_permissionManager;
    private FeatureManager m_featureManager;
    private LocationsManager m_locationsManager;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        TestHelper.cleanInsert("ClearDb.xml");
        sql("commserver/SeedLocations.sql");
        m_addressManager = EasyMock.createMock(AddressManager.class);
        m_addressManager.getSingleAddress(Ivr.SIP_ADDRESS);
        EasyMock.expectLastCall().andReturn(new Address(Ivr.SIP_ADDRESS, "localhost"));
        EasyMock.replay(m_addressManager);
    }

    @Override
    protected void onTearDownAfterTransaction() throws Exception {
        super.onTearDownAfterTransaction();
        TestHelper.cleanInsert("ClearDb.xml");
    }

    /*
     * Cannot make this test run consistently. Run from Eclipse it passes.
     * Run single from CLI it passes sometimes
     * In precommit it fails all the time.
     */
    public void testReplicateVmPermission() throws Exception {
/*        loadDataSetXml("domain/DomainSeed.xml");
        User user = m_coreContext.newUser();
        user.setUserName("user1");
        user.setFirstName("FirstName");
        user.setLastName("LastName");
        user.setPintoken("password");
        user.setSipPassword("sippassword");
        m_coreContext.saveUser(user);
        commit();
        Location l = m_locationsManager.getPrimaryLocation();
        m_featureManager.enableLocationFeature(Registrar.FEATURE, l, true);
        m_featureManager.enableLocationFeature(ProxyManager.FEATURE, l, true);
        m_featureManager.enableLocationFeature(Ivr.FEATURE, l, true);
        DBObject als = new BasicDBObject();
        als.put("id", "~~vm~user1");
        als.put("cnt", "<sip:IVR@vm.example.org;mailbox=user1;action=deposit;locale=en>");
        als.put("rln", "vmprm");
        List<DBObject> list = new ArrayList<DBObject>();
        list.add(als);
        DBObject userMongo = new BasicDBObject();
        userMongo.put("als", list);
        Thread.sleep(10000);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), userMongo);*/
    }

    @Override
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public void setSipxReplicationContextImpl(SipxReplicationContextImpl sipxReplicationContextImpl) {
        m_sipxReplicationContextImpl = sipxReplicationContextImpl;
    }

    @Override
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
