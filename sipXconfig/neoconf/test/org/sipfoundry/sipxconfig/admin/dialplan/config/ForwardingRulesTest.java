/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class ForwardingRulesTest extends XMLTestCase {
    protected void setUp() throws Exception {
        XmlUnitHelper.setNamespaceAware(false);
    }
    
    public void testGenerate() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.getHostPatterns();
        control.andReturn(new String[] {
            "gander"
        });

        control.replay();

        ForwardingRules rules = new ForwardingRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.begin();
        rules.generate(rule);
        rules.end();

        Document document = rules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);
        
        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[5]", domDoc);

        control.verify();
    }
}
