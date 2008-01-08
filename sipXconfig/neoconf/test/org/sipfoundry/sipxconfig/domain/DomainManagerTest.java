/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Collections;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.springframework.orm.hibernate3.HibernateTemplate;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.reset;
import static org.easymock.classextension.EasyMock.verify;

public class DomainManagerTest extends TestCase {

    public void testSaveDomain() {
        Domain domain = new Domain("goose");

        final SipxServer server = createMock(SipxServer.class);
        server.setDomainName("goose");
        server.setRegistrarDomainAliases(null);
        server.applySettings();

        HibernateTemplate db = createMock(HibernateTemplate.class);
        db.findByNamedQuery("domain");
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        db.saveOrUpdate(domain);
        db.flush();
        replay(server, db);
        // TEST DELETE EXISTING
        DomainManagerImpl mgr = new DomainManagerImpl() {
            public void replicateDomainConfig() {
                // do not replicate anything - it's just a test
            }
            
            protected DomainConfiguration createDomainConfiguration() {
                return new DomainConfiguration();
            }
            
            protected SipxServer getServer() {
                return server;
            }
        };
        mgr.setHibernateTemplate(db);
        mgr.saveDomain(domain);

        verify(server, db);

        reset(server, db);

        // TEST IGNORE EXISTING (assumes there is no other, doesn't care actually)
        domain.setUniqueId(); // isNew!
        server.setDomainName("goose");
        server.setRegistrarDomainAliases(null);
        server.applySettings();

        db.saveOrUpdate(domain);
        db.flush();        
        replay(server, db);

        mgr.saveDomain(domain);

        verify(server, db);
    }
}
