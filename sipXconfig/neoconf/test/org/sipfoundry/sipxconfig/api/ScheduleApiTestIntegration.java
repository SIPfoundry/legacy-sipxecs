package org.sipfoundry.sipxconfig.api;

import java.util.List;

import org.junit.Test;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class ScheduleApiTestIntegration extends RestApiIntegrationTestCase {
    private ForwardingContext m_forwardingContext;
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    @Test
    public void _testScheduleJsonApi() throws Exception {
        // query empty schedules
        String emptySchedules = getAsJson("schedules/general");
        assertEquals("{\"schedules\":[]}", emptySchedules);

        // create schedule
        String createSchedule =
            "{\"name\":\"Schedule1\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\","
            + "\"workingTime\":{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}";
        int code = postJsonString(createSchedule, "schedules");
        assertEquals(200, code);
        List<GeneralSchedule> genSchedules = m_forwardingContext.getAllGeneralSchedules();
        assertEquals(1, genSchedules.size());

        Schedule schedule = m_forwardingContext.getScheduleById(genSchedules.get(0).getId());

        // retrieve general schedules
        String schedules = getAsJson("schedules/general");
        assertEquals(
                String.format(
                        "{\"schedules\":[{\"id\":%s,\"name\":\"Schedule1\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\","
                        + "\"workingTime\":{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}]}", schedule.getId()), schedules);

        // retrieve schedule
        String scheduleJson = getAsJson(String.format("schedules/%s", schedule.getId()));
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"Schedule1\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\",\"workingTime\":"
                        + "{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}",
                        schedule.getId()), scheduleJson);

        // modify schedule
        String modifySchedule = "{\"name\":\"Schedule1Modified\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\","
            + "\"workingTime\":{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}";
        int putCode = putJsonString(modifySchedule, String.format("schedules/%s", schedule.getId()));
        assertEquals(200, putCode);

        //retrieve modified schedule
        scheduleJson = getAsJson(String.format("schedules/%s", schedule.getId()));
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"Schedule1Modified\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\",\"workingTime\":"
                        + "{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}", schedule.getId()), scheduleJson);
        //add period
        String period = "{\"enabled\":false,\"start\":34400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"}";
        int addPeriodCode = postJsonString(period,  String.format("schedules/%s/period", schedule.getId()));
        assertEquals(200, addPeriodCode);

        //retrieve schedule with period
        scheduleJson = getAsJson(String.format("schedules/%s", schedule.getId()));
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"Schedule1Modified\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\",\"workingTime\":"
                        + "{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"},"
                        + "{\"enabled\":false,\"start\":34400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"}]}}", schedule.getId()), scheduleJson);

        //delete period
        int deletePeriod = delete(String.format("schedules/%s/period/1", schedule.getId()));
        assertEquals(200, deletePeriod);
        //retrieve schedule without added period
        scheduleJson = getAsJson(String.format("schedules/%s", schedule.getId()));
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"Schedule1Modified\",\"description\":\"Description1\",\"userId\":-1,\"groupId\":-1,\"type\":\"G\",\"workingTime\":"
                        + "{\"workingHours\":[{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"}]}}", schedule.getId()), scheduleJson);

        // delete schedule
        int deleteSchedule = delete(String.format("schedules/%s", schedule.getId()));
        assertEquals(200, deleteSchedule);
        assertEquals(0, m_forwardingContext.getAllGeneralSchedules().size());
    }

    @Test
    public void testScheduleXmlApi() throws Exception {
        // query empty schedules
        String emptySchedules = getAsXml("schedules/general");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><Schedules/>", emptySchedules);

        // create schedule
        String createSchedule =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Schedule><name>Schedule1</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
            + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
            + "</workingHours></workingTime></Schedule>";
        int code = postXmlString(createSchedule, "schedules");
        assertEquals(200, code);
        List<GeneralSchedule> genSchedules = m_forwardingContext.getAllGeneralSchedules();
        assertEquals(1, genSchedules.size());

        Schedule schedule = m_forwardingContext.getScheduleById(genSchedules.get(0).getId());

        // retrieve general schedules
        String schedules = getAsXml("schedules/general");
        assertEquals(
                String.format(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Schedules><Schedule><id>%s</id><name>Schedule1</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
                        + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
                        + "</workingHours></workingTime></Schedule></Schedules>", schedule.getId()), schedules);

        // retrieve schedule
        String scheduleXml = getAsXml(String.format("schedules/%s", schedule.getId()));
        assertEquals(
                String.format(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Schedule><id>%s</id><name>Schedule1</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
                        + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
                        + "</workingHours></workingTime></Schedule>",
                        schedule.getId()), scheduleXml);

        // modify schedule
        String modifySchedule = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Schedule><name>Schedule1Modified</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
            + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
            + "</workingHours></workingTime></Schedule>";

        int putCode = putXmlString(modifySchedule, String.format("schedules/%s", schedule.getId()));
        assertEquals(200, putCode);

        //retrieve modified schedule
        scheduleXml = getAsXml(String.format("schedules/%s", schedule.getId()));
        assertEquals(String.format(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Schedule><id>%s</id><name>Schedule1Modified</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
                        + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
                        + "</workingHours></workingTime></Schedule>", schedule.getId()), scheduleXml);
        //add period
        String period = "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:04:00+02:00</stop>"
            + "<scheduledDay>MONDAY</scheduledDay></workingHours>";
        int addPeriodCode = postXmlString(period,  String.format("schedules/%s/period", schedule.getId()));
        assertEquals(200, addPeriodCode);

        //retrieve schedule with period
        scheduleXml = getAsXml(String.format("schedules/%s", schedule.getId()));
        assertEquals(String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Schedule><id>%s</id><name>Schedule1Modified</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
                        + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
                        + "</workingHours><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:04:00+02:00</stop>"
                        + "<scheduledDay>MONDAY</scheduledDay></workingHours></workingTime></Schedule>", schedule.getId()), scheduleXml);

        //delete period
        int deletePeriod = delete(String.format("schedules/%s/period/1", schedule.getId()));
        assertEquals(200, deletePeriod);
        //retrieve schedule without added period
        scheduleXml= getAsXml(String.format("schedules/%s", schedule.getId()));
        assertEquals(String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Schedule><id>%s</id><name>Schedule1Modified</name><description>Description1</description><userId>-1</userId><groupId>-1</groupId><type>general</type>"
                        + "<workingTime><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay>"
                        + "</workingHours></workingTime></Schedule>", schedule.getId()), scheduleXml);

        // delete schedule
        int deleteSchedule = delete(String.format("schedules/%s", schedule.getId()));
        assertEquals(200, deleteSchedule);
        assertEquals(0, m_forwardingContext.getAllGeneralSchedules().size());
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }
}
