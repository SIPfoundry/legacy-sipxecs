/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import junit.framework.TestCase;

import org.dom4j.Element;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class DataSetGeneratorTest extends TestCase {
    
    public void testGetDomain() {
        DataSetGenerator dsg = new DataSetGenerator() {
            protected DataSet getType() {
                return null;
            }

            protected void addItems(Element items) {
                // do nothing
            }
        };

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("core.domain.com");
        coreContextCtrl.replay();

        dsg.setCoreContext(coreContext);
        assertEquals("core.domain.com", dsg.getSipDomain());

        coreContextCtrl.verify();
        
        dsg.setSipDomain("set.domain.com");
        assertEquals("set.domain.com", dsg.getSipDomain());
    }
}
