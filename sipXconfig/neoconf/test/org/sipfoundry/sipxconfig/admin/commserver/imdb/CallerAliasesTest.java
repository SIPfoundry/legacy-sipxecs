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

import static org.easymock.EasyMock.replay;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

public class CallerAliasesTest extends MongoTestCase {

    public void testGenerate() throws Exception {
        CallerAliases cas = new CallerAliases();

        cas.setCoreContext(getCoreContext());
        replay(getCoreContext());
        cas.setDbCollection(getCollection());
        
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
        
        cas.generate(gw);
        
        MongoTestCaseHelper.assertObjectWithIdPresent("Gateway1");
        DBObject ref = new BasicDBObject();
        ref.put("id", "Gateway1");
        ref.put("ident", "gateway.example.org;sipxecs-lineid=1");
        ref.put("uid", "~~gw");
        ref.put(MongoConstants.CALLERALIAS, "\"display name\"<sip:gatewayCID@mydomain.org;key=value>");
        ref.put(MongoConstants.IGNORE_USER_CID, gcai.isIgnoreUserInfo());
        ref.put(MongoConstants.CID_PREFIX, gcai.getAddPrefix());
        ref.put(MongoConstants.KEEP_DIGITS, gcai.getKeepDigits());
        ref.put(MongoConstants.TRANSFORM_EXT, gcai.isTransformUserExtension());
        ref.put(MongoConstants.ANONYMOUS, gcai.isAnonymous());
        MongoTestCaseHelper.assertObjectPresent(ref);
        
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        User user = new User();
        user.setUniqueId(1);
        user.setPermissionManager(pm);
        user.setSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER, "userCID");
        
        cas.generate(user);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", MongoConstants.CALLERALIAS, "sip:userCID@mydomain.org");

        User userWithoutClrid = new User();
        userWithoutClrid.setUniqueId(1);
        userWithoutClrid.setPermissionManager(pm);
        
        cas.generate(userWithoutClrid);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent("User1", MongoConstants.CALLERALIAS, "");
        
    }
}
