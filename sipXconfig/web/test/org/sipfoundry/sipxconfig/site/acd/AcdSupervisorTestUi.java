/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.junit.WebTestCase;

public class AcdSupervisorTestUi extends WebTestCase {
    private static String[] NAVIGATION_LINKS = { 
        "link:agentPresence", 
        "link:agentsStats", 
        "link:callsStats", 
        "link:queuesStats" 
    };
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AcdSupervisorTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());        
        clickLink("resetAcdContext");
    }
    
    public void testDisplay() {
        clickLink("acdSupervisorConsole");        
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testNoServersMessage() {
        clickLink("acdSupervisorConsole");
        
        for (String link : NAVIGATION_LINKS) {
            clickLink(link);        
            assertElementPresent("message.noServers");
        }
    }
    
    public void testOneServer() {
        clickLink("acdServerPage");
        clickButton("form:apply");

        SiteTestHelper.home(getTester());        
        clickLink("acdSupervisorConsole");

        for (String link : NAVIGATION_LINKS) {
            clickLink(link);        
            assertElementNotPresent("message.selectServer");            
        }
        
        assertStatisticsDisplay();
    }
    
    private void logout() {
        SiteTestHelper.home(getTester(), false);         
        clickLink("Logout"); // clears acd server id in session                
        SiteTestHelper.home(getTester());         
    }
    
    public void testMultipleServers() {
        clickLink("acdServerPage");
        setTextField("hostField", "localhost0");
        clickButton("form:apply");

        logout(); // kludge edit page persists id, clear it

        clickLink("acdServerPage");
        setTextField("hostField", "localhost1");
        clickButton("form:apply");

        logout(); // clears acd server id in session

        clickLink("acdSupervisorConsole");

        for (String link : NAVIGATION_LINKS) {
            clickLink(link);        
            assertElementPresent("message.selectServer");
        }
        
        clickButton("button.selectServer");        
        assertStatisticsDisplay();
    }

    private void assertStatisticsDisplay() {
        clickLink(NAVIGATION_LINKS[0]);
        assertElementPresent("list.presence");
        clickLink(NAVIGATION_LINKS[1]);
        assertElementPresent("agent:stats:list");
        clickLink(NAVIGATION_LINKS[2]);
        assertElementPresent("calls:stats:list");
        clickLink(NAVIGATION_LINKS[3]);        
        assertElementPresent("queues:stats:list");
    }    
}
