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
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

public class CallerAliasesIntegrationTest extends ImdbTestCase {
    
    public void onSetupBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    public void testGenerate() throws Exception {
        CallerAliases cas = new CallerAliases();
        cas.setCoreContext(getCoreContext());
        cas.setDbCollection(getEntityCollection());
        
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
        
        cas.generate(gw, cas.findOrCreate(gw));
        
        assertObjectWithIdPresent("Gateway1");
        DBObject ref = new BasicDBObject();
        ref.put(MongoTestCaseHelper.ID, "Gateway1");
        ref.put("ident", "gateway.example.org;sipxecs-lineid=1");
        ref.put("uid", "~~gw");
        ref.put(MongoConstants.CALLERALIAS, "\"display name\"<sip:gatewayCID@example.org;key=value>");
        ref.put(MongoConstants.IGNORE_USER_CID, gcai.isIgnoreUserInfo());
        ref.put(MongoConstants.CID_PREFIX, gcai.getAddPrefix());
        ref.put(MongoConstants.KEEP_DIGITS, gcai.getKeepDigits());
        ref.put(MongoConstants.TRANSFORM_EXT, gcai.isTransformUserExtension());
        ref.put(MongoConstants.ANONYMOUS, gcai.isAnonymous());
        assertObjectPresent(ref);
        
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setUniqueId(1);
        user.setPermissionManager(pm);
        user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, "userCID");
        
        cas.generate(user, cas.findOrCreate(user));
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.CALLERALIAS, "sip:userCID@example.org");

        User userWithoutClrid = new User();
        userWithoutClrid.setUniqueId(1);
        userWithoutClrid.setPermissionManager(pm);
        
        cas.generate(userWithoutClrid, cas.findOrCreate(userWithoutClrid));
        assertObjectWithIdFieldValuePresent("User1", MongoConstants.CALLERALIAS, "");       
    }
}
