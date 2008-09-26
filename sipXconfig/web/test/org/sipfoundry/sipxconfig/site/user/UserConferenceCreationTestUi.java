package org.sipfoundry.sipxconfig.site.user;

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.conference.ConferenceTestHelper;

public class UserConferenceCreationTestUi extends WebTestCase {

    private ConferenceTestHelper m_helper;
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserConferenceCreationTestUi.class);
    }    
    
    @Override
    protected void setUp() {
        m_helper = new ConferenceTestHelper(tester);
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        SiteTestHelper.setScriptingEnabled(tester, true);      
        clickLink("resetCoreContext");
        clickLink("resetConferenceBridgeContext");
    }
    
    public void testConferenceCreation() {
        // Create the test bridge and a user group for conference creation.
        m_helper.createBridge("testbridge");
        SiteTestHelper.seedGroup(getTester(), "NewUserGroup", 1);
        
        // Enable conference creation and select the previously created bridge.
        clickLink("UserGroups");
        clickLinkWithText("seedGroup0");         
        clickLink("link:conferences");               
        checkCheckbox("conferences:enable");
        setTextField("conferences:offset", "1000");
        selectOption("bridgeSelect", "testbridge");
        submit("submit:ok");

        // Create a new user and assign it to the group.
        SiteTestHelper.home(tester);
        clickLink("NewUser");
        setTextField("user:userId", "300");
        setTextField("cp:password", "12345");
        setTextField("cp:confirmPassword", "12345");
        setTextField("gms:groups", "seedGroup0");
        submit("form:apply");
        
        // Lastly, verify that a conference was created for this user.
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        clickLinkWithText("testbridge");
        clickLink("link:conferences");
        Table conferenceTable = getTable("conference:list");
        assertEquals(3, conferenceTable.getRowCount()); // the header, the pager and the one conference
        assertEquals("300-conference", SiteTestHelper.getCellAsText(conferenceTable, 2, 1)); // conference name
        assertEquals("300", SiteTestHelper.getCellAsText(conferenceTable, 2, 2)); // owner username
        assertEquals("1300", SiteTestHelper.getCellAsText(conferenceTable, 2, 4)); // offset + extension
    }
    
}
