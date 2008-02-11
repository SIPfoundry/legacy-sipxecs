/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivatedEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.springframework.context.ApplicationEvent;

public class LazySipxReplicationContextImplTest extends TestCase {

    public void testGenerateAll() throws Exception {
        int lazyIterations = 20;
        XmlFile mr = new MappingRules();
        XmlFile orbits = new Orbits();
        
        ApplicationEvent event = new DialPlanActivatedEvent(this);
       
        LazySipxReplicationContextImpl lazy = new LazySipxReplicationContextImpl();
        
        IMocksControl replicationCtrl = EasyMock.createControl();
        SipxReplicationContext replication = replicationCtrl.createMock(SipxReplicationContext.class);
        replication.replicate(mr);
        replication.generate(DataSet.ALIAS);
        replication.generate(DataSet.CREDENTIAL);
        replication.generate(DataSet.EXTENSION);
        replication.generate(DataSet.PERMISSION);
        replication.generate(DataSet.CALLER_ALIAS);
        replication.publishEvent(event);
        replication.replicate(orbits);
        replication.generate(DataSet.ALIAS);
        replication.generate(DataSet.CREDENTIAL);
        replication.generate(DataSet.EXTENSION);
        replication.generate(DataSet.PERMISSION);
        replication.generate(DataSet.CALLER_ALIAS);
        replicationCtrl.replay();
        
        int interval = 50;
        lazy.setSleepInterval(interval);
        lazy.setTarget(replication);
        
        lazy.init();
        
        lazy.replicate(mr);
        for(int i = 0; i < lazyIterations; i++) {
            lazy.generate(DataSet.ALIAS);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }
        lazy.publishEvent(event);
        
        Thread.sleep(400);
        
        lazy.replicate(orbits);
        for(int i = 0; i < lazyIterations; i++) {
            lazy.generate(DataSet.ALIAS);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }        

        Thread.sleep(1600);
        
        replicationCtrl.verify();
    }
}
