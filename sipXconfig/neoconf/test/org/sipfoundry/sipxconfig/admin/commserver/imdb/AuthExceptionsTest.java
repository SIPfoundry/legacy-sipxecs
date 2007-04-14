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

import java.util.Arrays;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;

public class AuthExceptionsTest extends XMLTestCase {
    private String[] DATA = { "aaa", "bbb", "ccc" };

    public void testGenerate() throws Exception {
        IMocksControl control = EasyMock.createControl();
        ForwardingContext forwardingContext = control.createMock(ForwardingContext.class);
        forwardingContext.getForwardingAuthExceptions();
        control.andReturn(Arrays.asList(DATA));
        control.replay();

        AuthExceptions exceptions = new AuthExceptions();
        exceptions.setForwardingContext(forwardingContext);

        Document document = exceptions.generate();        
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("authexception", "/items/@type", domDoc);
        for (int i = 0; i < DATA.length; i++) {
            assertXpathEvaluatesTo(DATA[i], "/items/item[" + (i + 1) + "]/user", domDoc);            
        }
        control.verify();
    }    
}
