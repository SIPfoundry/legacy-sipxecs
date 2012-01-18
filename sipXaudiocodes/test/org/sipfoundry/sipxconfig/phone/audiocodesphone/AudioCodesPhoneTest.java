package org.sipfoundry.sipxconfig.phone.audiocodesphone;

import static org.easymock.classextension.EasyMock.replay;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.audiocodesphone.AudioCodesPhone.SpeedDialProfile;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AudioCodesPhoneTest extends TestCase {

    private AudioCodesPhone m_phone;
    private PhoneModel m_model;
    private Location m_primaryLocation;
    private LocationsManager m_locationsManager;

    @Override
    protected void setUp() throws Exception {
        m_phone = new AudioCodesPhone();
        m_phone.setModelFilesContext(TestHelper.getModelFilesContext());
        m_model = new PhoneModel("audiocodesphone");
        m_phone.setModel(m_model);
        m_phone.setModelId("audiocodes-320");
        m_model.setProfileTemplate("audiocodesphone/mac.cfg.vm");
        m_model.setMaxLineCount(4);
        m_primaryLocation = TestHelper.createDefaultLocation();
        
        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        m_locationsManager.getLocations();
        EasyMock.expectLastCall().andReturn(new Location[] {m_primaryLocation}).anyTimes();
        m_locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(m_primaryLocation).atLeastOnce();
        
        m_primaryLocation.setFqdn("example.com");
        m_phone.setLocationsManager(m_locationsManager);

        replay(m_locationsManager);
    }

    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("audiocodesphone")));
    }

    public void testGetFileName() throws Exception {
        m_phone.setModelId("audiocodes-320");
        m_phone.setSerialNumber("0011aabb4455");
        assertEquals("0011aabb4455.cfg", m_phone.getProfileFilename());
    }

    public void testRestart() throws Exception {
        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(m_phone,true,false,false,true);
        m_phone.restart();

        testDriver.getSipControl().verify();
    }

    public void testRestartNoLine() throws Exception {
        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(m_phone, new ArrayList<User>(), true);
        m_phone.restart();
        testDriver.getSipControl().verify();
    }

    /**
     * Tests that the SpeedDialProfile is used when phonebook management is enabled.
     */
    public void testPhonebookManagementEnabled() throws Exception {
        PhoneTestDriver.supplyTestData(m_phone, true);

        // Should return two profiles - the regular profile and the speeddial
        // profile.
        Profile[] profileTypes = m_phone.getProfileTypes();
        assertEquals(2, profileTypes.length);
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
        assertTrue(profileTypes[1].getClass().equals(SpeedDialProfile.class));
    }

    /**
     * Tests that the SpeedDialProfile is not used when phonebook management is disabled.
     */
    public void testPhonebookManagementDisabled() throws Exception {
        PhoneTestDriver.supplyTestData(m_phone, false);

        // Should only return one Profile.
        Profile[] profileTypes = m_phone.getProfileTypes();
        assertEquals(1, profileTypes.length);
        // Make sure it's not a PhonebookProfile. We can't use instanceof to
        // check the type, because since a PhonebookProfile is a Profile, the
        // result would be true. So we have to compare the classes directly.
        assertTrue(profileTypes[0].getClass().equals(Profile.class));
    }

    public void testGenerateTypicalProfile() throws Exception {
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_phone, TestHelper.getEtcDir());

        supplyTestData(m_phone);
        m_phone.getProfileTypes()[0].generate(m_phone, location);
        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }

    private void supplyTestData(AudioCodesPhone phone) {
        User u1 = new User();
        u1.setUserName("200");
        u1.setSipPassword("1234");

        User u2 = new User();
        u2.setUserName("201");
        u2.setSipPassword("1234");

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(new User[] {
            u1, u2
        }));
    }
}
