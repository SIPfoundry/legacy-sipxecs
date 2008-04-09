/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.easymock.EasyMock;
import org.easymock.IArgumentMatcher;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class ConferenceBridgeProvisioningtImplTestDb extends SipxDatabaseTestCase {

    private ConferenceBridgeContext m_context;

    protected void setUp() throws Exception {
        m_context = (ConferenceBridgeContext) TestHelper.getApplicationContext().getBean(
                ConferenceBridgeContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("conference/users.db.xml");
    }
    
    static Hashtable settingsMapMatcher() {
        EasyMock.reportMatcher(new SettingMatcher());
        return null;
    }

    public void testDeploy() throws Exception {
        IMocksControl dbCtrl = EasyMock.createControl();
        ConfigDbParameter db = dbCtrl.createMock(ConfigDbParameter.class);
        db.set(EasyMock.eq("bbridge.conf"), settingsMapMatcher());
        
        // do not check params for now - we just verify that the function has been called once
        dbCtrl.andReturn(10);

        dbCtrl.replay();

        ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
        adaptor.setConfigDbParameter(db);

        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2005));

        IMocksControl hibernateCtrl = org.easymock.classextension.EasyMock.createControl();
        HibernateTemplate hibernate = hibernateCtrl.createMock(HibernateTemplate.class);
        hibernate.saveOrUpdateAll(bridge.getConferences());
        hibernateCtrl.replay();

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(hibernate);

//        impl.deploy(bridge, adaptor);

        dbCtrl.verify();
        hibernateCtrl.verify();
    }

    public void testGenerateAdmissionData() throws Exception {
        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2005));

        IMocksControl hibernateCtrl = org.easymock.classextension.EasyMock.createControl();
        HibernateTemplate hibernate = hibernateCtrl.createMock(HibernateTemplate.class);
        hibernate.loadAll(Conference.class);
        hibernateCtrl.andReturn(new ArrayList(bridge.getConferences()));
        hibernateCtrl.replay();

        IMocksControl replicationCtrl = EasyMock.createControl();
        SipxReplicationContext replication = replicationCtrl.createMock(SipxReplicationContext.class);
        EasyMock.anyObject();
        replication.replicate(null);
        replicationCtrl.replay();

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.setHibernateTemplate(hibernate);
        impl.setSipxReplicationContext(replication);
        
        List conferences = hibernate.loadAll(Conference.class);

        impl.generateAdmissionData(bridge, conferences);

        replicationCtrl.verify();
        hibernateCtrl.verify();
    }
    
    static class AlwaysMatcher implements IArgumentMatcher {

        public boolean matches(Object argument) {
            return true;
        }

        public void appendTo(StringBuffer buffer) {
        }        
    }

    static class SettingMatcher implements IArgumentMatcher {
        
//        protected boolean argumentMatches(Object expected, Object actual) {
//            if (!(expected instanceof Map)) {
//                return super.argumentMatches(expected, actual);
//            }
//            Map args = (Map) expected;
//            // uncomment to see parameters sent to XML/RPC
//            // MapUtils.debugPrint(System.err, "expected:", args);
//            for (Iterator i = args.keySet().iterator(); i.hasNext();) {
//                String name = (String) i.next();
//                // all settings should start with the same prefix
//                if (!name.startsWith("BOSTON_BRIDGE")) {
//                    return false;
//                }
//            }
//            return true;
//        }

        public boolean matches(Object argument) {
            Map actual = (Map) argument;
            // uncomment to see parameters sent to XML/RPC
            // MapUtils.debugPrint(System.err, "expected:", args);
            for (Iterator i = actual.keySet().iterator(); i.hasNext();) {
                String name = (String) i.next();
                // all settings should start with the same prefix
                if (!name.startsWith("BOSTON_BRIDGE")) {                   
                    return false;
                }
            }
            
            return true;
        }

        public void appendTo(StringBuffer buffer) {
            buffer.append("all settings should start with the same prefix");
        }
    }
}
