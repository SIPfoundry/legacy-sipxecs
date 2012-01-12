/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;

import com.mongodb.DBObject;

public class CallerAliasesTestIntegration extends ImdbTestCase {
    private CallerAliases m_calleraliasDataSet;
    private ReplicationManagerImpl m_replManager;
    
    public void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    public void testGenerate() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        GatewayCallerAliasInfo gcai = new GatewayCallerAliasInfo();
        gcai.setDefaultCallerAlias("gatewayCID");
        
        Gateway gw = new Gateway();
        gw.setAddress("gateway.example.org");
        gw.setUniqueId(1);
        gcai.setAddPrefix("111");
        gcai.setIgnoreUserInfo(false);
        gcai.setKeepDigits(0);
        gcai.setTransformUserExtension(false);
        gcai.setAnonymous(true);
        gcai.setDisplayName("display name");
        gcai.setUrlParameters("key=value");
        gw.setCallerAliasInfo(gcai);

        DBObject gwObj = m_replManager.findOrCreate(gw);
        m_calleraliasDataSet.generate(gw, gwObj);
        assertEquals("gateway.example.org;sipxecs-lineid=1", gwObj.get("ident"));
        assertEquals("~~gw", gwObj.get("uid"));
        assertEquals("\"display name\"<sip:gatewayCID@example.org;key=value>", gwObj.get(MongoConstants.CALLERALIAS));
        assertEquals(false, gwObj.get(MongoConstants.IGNORE_USER_CID));
        assertEquals("111", gwObj.get(MongoConstants.CID_PREFIX));
        assertEquals(0, gwObj.get(MongoConstants.KEEP_DIGITS));
        assertEquals(false, gwObj.get(MongoConstants.TRANSFORM_EXT));
        assertEquals(true, gwObj.get(MongoConstants.ANONYMOUS));
        
        User user = new User();
        user.setUniqueId(1);
        user.setPermissionManager(getPermissionManager());
        user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, "userCID");

        DBObject userObj = m_replManager.findOrCreate(user);
        m_calleraliasDataSet.generate(user, userObj);
        assertEquals("sip:userCID@example.org", userObj.get(MongoConstants.CALLERALIAS));

        User userWithoutClrid = new User();
        userWithoutClrid.setUniqueId(1);
        userWithoutClrid.setPermissionManager(getPermissionManager());

        DBObject userWithoutClridObj = m_replManager.findOrCreate(user);
        m_calleraliasDataSet.generate(userWithoutClrid, userWithoutClridObj);
        assertEquals("", userWithoutClridObj.get(MongoConstants.CALLERALIAS));
    }

    public void setCalleraliasDataSet(CallerAliases calleraliasDataSet) {
        m_calleraliasDataSet = calleraliasDataSet;
    }

    public void setReplicationManagerImpl(ReplicationManagerImpl replManager) {
        m_replManager = replManager;
    }
}
