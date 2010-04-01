package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;

public class DimDimConferenceTest extends TestCase {
    private Conference m_conference;

    @Override
    protected void setUp() throws Exception {
        m_conference = new Conference();
        m_conference.setName("my_conf");
        m_conference.setExtension("12345");
        m_conference.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testGetCreateMeetingUrlEmpty() {
        DimDimConference dimDimConference = new DimDimConference(m_conference);
        assertFalse(dimDimConference.isConfigured());
        assertNull(dimDimConference.getCreateMeetingUrl());
        assertNull(dimDimConference.getJoinMeetingUrl());
    }

    public void testGetCreateMeetingUrl() {
        DimDimConference dimDimConference = new DimDimConference(m_conference);

        m_conference.setSettingTypedValue("web-meeting/dimdim-host", "my.dimdim.com");
        m_conference.setSettingTypedValue("web-meeting/did", "6131234567");
        m_conference.setSettingTypedValue("web-meeting/user", "dimUser");
        m_conference.setSettingTypedValue("web-meeting/password", "dimPass");

        assertTrue(dimDimConference.isConfigured());
        assertEquals(
                "http://my.dimdim.com/portal/start.action?name=dimUser&password=dimPass&confname=my_conf&internToll=12345&internToll=6131234567",
                dimDimConference.getCreateMeetingUrl());
        assertEquals("http://my.dimdim.com/portal/JoinForm.action?meetingRoomName=dimUser", dimDimConference
                .getJoinMeetingUrl());
    }

    public void testGetCreateMeetingUrlWithOwner() {
        User user = new User();
        user.setUserName("jadams");
        user.setFirstName("John");
        user.setLastName("Adams");

        DimDimConference dimDimConference = new DimDimConference(m_conference);

        m_conference.setOwner(user);
        m_conference.setSettingTypedValue("web-meeting/password", "dimPass");
        m_conference.setSettingTypedValue(Conference.PARTICIPANT_CODE, "9988");
        m_conference.setSettingTypedValue("web-meeting/did", "6131234567");

        assertTrue(dimDimConference.isConfigured());
        assertEquals(
                "http://webmeeting.dimdim.com/portal/start.action?name=jadams&password=dimPass&confname=my_conf&internToll=12345&internToll=6131234567&attendeePasscode=9988&attendeePwd=9988&displayname=John+Adams",
                dimDimConference.getCreateMeetingUrl());
        assertEquals("http://webmeeting.dimdim.com/portal/JoinForm.action?meetingRoomName=jadams&attendeePasscode=9988&attendeePwd=9988",
                dimDimConference.getJoinMeetingUrl());
    }
}
