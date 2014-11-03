package org.sipfoundry.sipxconfig.api;

import java.text.SimpleDateFormat;
import java.util.List;

import org.junit.Test;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialPlanSetup;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.test.RestApiIntegrationTestCase;

public class DialPlanApiTestIntegration extends RestApiIntegrationTestCase {

    private DialPlanContext m_dialPlanContext;
    private DialPlanSetup m_dialPlanSetup;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        m_dialPlanSetup.setupDefaultRegion();
    }

    @Test
    public void testDialPlanJsonApi() throws Exception {
        // query raw rules
        String rawRules = getAsJson("rules/raw");
        assertEquals("{\"names\":[\"internalRule\",\"defaultCustomRule\",\"longDistanceRule\",\"localRule\","
            + "\"emergencyRule\",\"internationalRule\",\"attendantRule\",\"siteToSiteRule\"]}", rawRules);

        //query rules
        List<DialingRule> rules = m_dialPlanContext.getRules();
        String rulesJson = getAsJson(String.format("rules"));
        assertEquals(
            String.format("{\"rules\":[{\"id\":%s,\"name\":\"Emergency\",\"enabled\":false,\"type\":"
            + "\"Emergency\",\"description\":\"Emergency dialing plan\",\"scheduleId\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,"
            + "\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":\"9\",\"emergencyNumber\":\"911\",\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,"
            + "\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,"
            + "\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"International\",\"enabled\":false,\"type\":\"Long_Distance\","
            + "\"description\":\"International dialing\",\"scheduleId\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,"
            + "\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"011\",\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":\"\",\"externalLen\":-1,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,"
            + "\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,"
            + "\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"Local\",\"enabled\":false,\"type\":\"Long_Distance\",\"description\":\"Local dialing\","
            + "\"scheduleId\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"9\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\",\"longDistancePrefixOptional\":true,"
            + "\"areaCodes\":\"\",\"externalLen\":7,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,"
            + "\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},"
            + "{\"id\":%s,\"name\":\"Long Distance\",\"enabled\":false,\"type\":\"Long_Distance\",\"description\":\"Long distance dialing plan\",\"scheduleId\":null,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,"
            + "\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"9\",\"pstnPrefixOptional\":true,\"longDistancePrefix\":\"1\",\"longDistancePrefixOptional\":false,"
            + "\"areaCodes\":\"\",\"externalLen\":10,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,"
            + "\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,"
            + "\"name\":\"Restricted\",\"enabled\":false,\"type\":\"Long_Distance\",\"description\":\"Restricted dialing\",\"scheduleId\":null,\"permissions\":{\"names\":[]},"
            + "\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,"
            + "\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\",\"longDistancePrefixOptional\":false,\"areaCodes\":\"900\","
            + "\"externalLen\":0,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,"
            + "\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},{\"id\":%s,\"name\":\"Toll free\",\"enabled\":false,"
            + "\"type\":\"Long_Distance\",\"description\":\"Toll free dialing\",\"scheduleId\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,"
            + "\"mediaServerHostname\":null,\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"\","
            + "\"longDistancePrefixOptional\":false,\"areaCodes\":\"800, 866, 877, 888\",\"externalLen\":0,\"optionalPrefix\":null,\"emergencyNumber\":null,"
            + "\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,\"extension\":null,"
            + "\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false},"
            + "{\"id\":%s,\"name\":\"AutoAttendant\",\"enabled\":true,\"type\":\"Attendant\",\"description\":\"Default autoattendant dialing plan\",\"scheduleId\":null,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,\"internal\":true,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,\"longDistancePrefix\":null,"
            + "\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,"
            + "\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":{\"holidayPeriods\":[]},"
            + "\"workingTimeAttendantPeriods\":{\"workingHours\":[{\"enabled\":true,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Monday\"},"
            + "{\"enabled\":true,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Tuesday\"},"
            + "{\"enabled\":true,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Wednesday\"},"
            + "{\"enabled\":true,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Thursday\"},"
            + "{\"enabled\":true,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Friday\"},"
            + "{\"enabled\":false,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Saturday\"},"
            + "{\"enabled\":false,\"start\":%s,\"stop\":%s,\"scheduledDay\":\"Sunday\"}]},"
            + "\"extension\":\"100\",\"attendantAliases\":\"operator 0\",\"did\":null,\"enableLiveAttendant\":true},"
            + "{\"id\":%s,\"name\":\"Voicemail\",\"enabled\":true,\"type\":\"Internal\",\"description\":\"Default voicemail dialing plan\","
            + "\"scheduleId\":null,\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,"
            + "\"internal\":true,\"mediaServerHostname\":null,\"mediaServerType\":\"freeswitchMediaServer\","
            + "\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,"
            + "\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,"
            + "\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,"
            + "\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,\"workingTimeAttendantPeriods\":null,"
            + "\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false}]}",
            rules.get(0).getId(), rules.get(1).getId(), rules.get(2).getId(), rules.get(3).getId(), rules.get(4).getId(), rules.get(5).getId(), rules.get(6).getId(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[0].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[0].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[1].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[1].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[2].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[2].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[3].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[3].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[4].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[4].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[5].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[5].getStop().getTime(),
            ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[6].getStart().getTime(), ((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[6].getStop().getTime(),
            rules.get(7).getId()),
            rulesJson);
        // create attendant dialing rule
        String createRule = "{\"type\":\"Attendant\",\"name\":\"Attedant-2\",\"enabled\":true,\"description\":\"didianara\",\"scheduleId\":null,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,\"internal\":true,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,"
            + "\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":null,"
            + "\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"holidayAttendant\":\"After hours\",\"workingTimeAttendant\":\"After hours\","
            + "\"holidayAttendantPeriods\":{\"holidayPeriods\":[{\"startDate\":1407877200000,\"endDate\":1407963600000}]},"
            + "\"workingTimeAttendantPeriods\":{\"workingHours\":[{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"},"
            + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Tuesday\"},{\"enabled\":true,\"start\":32400000,\"stop\":64800000,"
            + "\"scheduledDay\":\"Wednesday\"},{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"},{\"enabled\":false,"
            + "\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Friday\"},{\"enabled\":false,\"start\":32400000,\"stop\":64800000,"
            + "\"scheduledDay\":\"Saturday\"},{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Sunday\"}]},\"extension\":\"1002\","
            + "\"attendantAliases\":\"zzzz 2\",\"did\":null,\"enableLiveAttendant\":true}";
        int code = postJsonString(createRule, "rules");
        assertEquals(200, code);

        int savedRuleId = db().queryForInt("select dialing_rule_id from dialing_rule where name='Attedant-2'");
        //create long distance dialing rule
        createRule = "{\"type\":\"Long_Distance\",\"name\":\"LD\",\"enabled\":false,\"description\":\"didi\",\"scheduleId\":2,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":true,\"authorizationChecked\":true,\"internal\":false,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":\"9\",\"pstnPrefixOptional\":false,\"longDistancePrefix\":\"1\","
            + "\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":10,\"optionalPrefix\":null,\"emergencyNumber\":null,\"afterHoursAttendant\":null,"
            + "\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":null,\"workingTimeAttendant\":null,\"holidayAttendantPeriods\":null,"
            + "\"workingTimeAttendantPeriods\":null,\"extension\":null,\"attendantAliases\":null,\"did\":null,\"enableLiveAttendant\":false}";
        code = postJsonString(createRule, "rules");
        assertEquals(200, code);

        //retrieve saved rule
        String ruleJson = getAsJson(String.format("rules/%s", savedRuleId));
        assertEquals(
                String.format(
                        "{\"id\":%s,\"name\":\"Attedant-2\",\"enabled\":true,\"type\":\"Attendant\",\"description\":\"didianara\",\"scheduleId\":null,"
                        + "\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,\"internal\":true,\"mediaServerHostname\":null,"
                        + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,"
                        + "\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":null,"
                        + "\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":\"After hours\","
                        + "\"workingTimeAttendant\":\"After hours\",\"holidayAttendantPeriods\":{\"holidayPeriods\":"
                        + "[{\"startDate\":1407877200000,\"endDate\":1407963600000}]},\"workingTimeAttendantPeriods\":{\"workingHours\":"
                        + "[{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Tuesday\"},"
                        + "{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Wednesday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Friday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Saturday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Sunday\"}]},"
                        + "\"extension\":\"1002\",\"attendantAliases\":\"zzzz 2\",\"did\":null,\"enableLiveAttendant\":true}",
                        savedRuleId), ruleJson);
        // modify rule
        String modifyRule = "{\"name\":\"Attedant-2-modified\",\"enabled\":true,\"type\":\"Attendant\",\"description\":\"didianara\",\"scheduleId\":null,"
            + "\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,\"internal\":true,\"mediaServerHostname\":null,"
            + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,"
            + "\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":null,"
            + "\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":\"After hours\","
            + "\"workingTimeAttendant\":\"Operator\",\"holidayAttendantPeriods\":{\"holidayPeriods\":"
            + "[{\"startDate\":1407877200000,\"endDate\":1407963600000}]},\"workingTimeAttendantPeriods\":{\"workingHours\":"
            + "[{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"},"
            + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Tuesday\"},"
            + "{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Wednesday\"},"
            + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"},"
            + "{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Friday\"},"
            + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Saturday\"},"
            + "{\"enabled\":true,\"start\":32500000,\"stop\":64800000,\"scheduledDay\":\"Sunday\"}]},"
            + "\"extension\":\"10025\",\"attendantAliases\":\"zzzz 2\",\"did\":null,\"enableLiveAttendant\":true}";
        int putCode = putJsonString(modifyRule, String.format("rules/%s", savedRuleId));
        assertEquals(200, putCode);

        //retrieve modified rule
        int count = db().queryForInt("select count(*) from dialing_rule where name='Attedant-2-modified'");
        assertEquals(1, count);
        ruleJson = getAsJson(String.format("rules/%s", savedRuleId));
        assertEquals(
                String.format(
                    "{\"id\":%s,\"name\":\"Attedant-2-modified\",\"enabled\":true,\"type\":\"Attendant\",\"description\":\"didianara\",\"scheduleId\":null,"
                        + "\"permissions\":{\"names\":[]},\"gatewayAware\":false,\"authorizationChecked\":true,\"internal\":true,\"mediaServerHostname\":null,"
                        + "\"mediaServerType\":null,\"dialPatterns\":null,\"callPattern\":null,\"pstnPrefix\":null,\"pstnPrefixOptional\":false,"
                        + "\"longDistancePrefix\":null,\"longDistancePrefixOptional\":false,\"areaCodes\":null,\"externalLen\":0,\"optionalPrefix\":null,"
                        + "\"emergencyNumber\":null,\"afterHoursAttendant\":null,\"afterHoursAttendantEnabled\":false,\"holidayAttendant\":\"After hours\","
                        + "\"workingTimeAttendant\":\"Operator\",\"holidayAttendantPeriods\":{\"holidayPeriods\":"
                        + "[{\"startDate\":1407877200000,\"endDate\":1407963600000}]},\"workingTimeAttendantPeriods\":{\"workingHours\":"
                        + "[{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Monday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Tuesday\"},"
                        + "{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Wednesday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Thursday\"},"
                        + "{\"enabled\":true,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Friday\"},"
                        + "{\"enabled\":false,\"start\":32400000,\"stop\":64800000,\"scheduledDay\":\"Saturday\"},"
                        + "{\"enabled\":true,\"start\":32500000,\"stop\":64800000,\"scheduledDay\":\"Sunday\"}]},"
                        + "\"extension\":\"10025\",\"attendantAliases\":\"zzzz 2\",\"did\":null,\"enableLiveAttendant\":true}",
                        savedRuleId), ruleJson);
        //delete rule
        int deleteRule = delete(String.format("rules/%s", savedRuleId));
        assertEquals(200, deleteRule);
        count = db().queryForInt("select count(*) from dialing_rule where name='Attedant-2-modified'");
        assertEquals(0, count);
    }

    @Test
    public void testDialPlanXmlApi() throws Exception {
        // query raw rules
        String rawRules = getAsXml("rules/raw");
        assertEquals("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Names><Name>internalRule</Name><Name>defaultCustomRule</Name>"
            + "<Name>longDistanceRule</Name><Name>localRule</Name><Name>emergencyRule</Name>"
            + "<Name>internationalRule</Name><Name>attendantRule</Name>"
            + "<Name>siteToSiteRule</Name></Names>", rawRules);
        //query rules
        List<DialingRule> rules = m_dialPlanContext.getRules();
        String rulesXml = getAsXml(String.format("rules"));
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSXXX");
        assertEquals(
            String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                + "<Rules>"
                + "<Rule><id>%s</id><name>Emergency</name><enabled>false</enabled><type>Emergency</type>"
                + "<description>Emergency dialing plan</description><permissions/><gatewayAware>true</gatewayAware>"
                + "<authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefixOptional>false</pstnPrefixOptional>"
                + "<longDistancePrefixOptional>false</longDistancePrefixOptional><externalLen>0</externalLen><optionalPrefix>9</optionalPrefix>"
                + "<emergencyNumber>911</emergencyNumber><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id><name>International</name>"
                + "<enabled>false</enabled><type>Long_Distance</type><description>International dialing</description><permissions/>"
                + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal>"
                + "<pstnPrefix></pstnPrefix><pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix>011</longDistancePrefix>"
                + "<longDistancePrefixOptional>false</longDistancePrefixOptional><areaCodes></areaCodes>"
                + "<externalLen>-1</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule>"
                + "<Rule><id>%s</id><name>Local</name><enabled>false</enabled><type>Long_Distance</type><description>Local dialing</description><permissions/>"
                + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix>9</pstnPrefix>"
                + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix>"
                + "<longDistancePrefixOptional>true</longDistancePrefixOptional>"
                + "<areaCodes></areaCodes><externalLen>7</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id>"
                + "<name>Long Distance</name><enabled>false</enabled><type>Long_Distance</type>"
                + "<description>Long distance dialing plan</description><permissions/><gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked>"
                + "<internal>false</internal><pstnPrefix>9</pstnPrefix><pstnPrefixOptional>true</pstnPrefixOptional><longDistancePrefix>1</longDistancePrefix>"
                + "<longDistancePrefixOptional>false</longDistancePrefixOptional><areaCodes></areaCodes><externalLen>10</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id><name>Restricted</name><enabled>false</enabled><type>Long_Distance</type>"
                + "<description>Restricted dialing</description><permissions/><gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked>"
                + "<internal>false</internal><pstnPrefix></pstnPrefix><pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix>"
                + "<longDistancePrefixOptional>false</longDistancePrefixOptional><areaCodes>900</areaCodes><externalLen>0</externalLen>"
                + "<afterHoursAttendantEnabled>false</afterHoursAttendantEnabled><enableLiveAttendant>false</enableLiveAttendant></Rule><Rule>"
                + "<id>%s</id><name>Toll free</name><enabled>false</enabled><type>Long_Distance</type><description>Toll free dialing</description><permissions/>"
                + "<gatewayAware>true</gatewayAware><authorizationChecked>true</authorizationChecked><internal>false</internal><pstnPrefix></pstnPrefix>"
                + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefix></longDistancePrefix><longDistancePrefixOptional>false</longDistancePrefixOptional>"
                + "<areaCodes>800, 866, 877, 888</areaCodes><externalLen>0</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule><Rule><id>%s</id><name>AutoAttendant</name><enabled>true</enabled><type>Attendant</type>"
                + "<description>Default autoattendant dialing plan</description><permissions/><gatewayAware>false</gatewayAware><authorizationChecked>true</authorizationChecked>"
                + "<internal>true</internal><pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefixOptional>false</longDistancePrefixOptional>"
                + "<externalLen>0</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled><holidayAttendantPeriods/>"
                + "<workingTimeAttendantPeriods><workingHours><enabled>true</enabled><start>%s</start>"
                + "<stop>%s</stop><scheduledDay>MONDAY</scheduledDay></workingHours><workingHours>"
                + "<enabled>true</enabled><start>%s</start><stop>%s</stop><scheduledDay>TUESDAY</scheduledDay></workingHours>"
                + "<workingHours><enabled>true</enabled><start>%s</start><stop>%s</stop><scheduledDay>WEDNESDAY</scheduledDay></workingHours>"
                + "<workingHours><enabled>true</enabled><start>%s</start><stop>%s</stop><scheduledDay>THURSDAY</scheduledDay></workingHours>"
                + "<workingHours><enabled>true</enabled><start>%s</start><stop>%s</stop><scheduledDay>FRIDAY</scheduledDay></workingHours>"
                + "<workingHours><enabled>false</enabled><start>%s</start><stop>%s</stop><scheduledDay>SATURDAY</scheduledDay></workingHours>"
                + "<workingHours><enabled>false</enabled><start>%s</start><stop>%s</stop><scheduledDay>SUNDAY</scheduledDay>"
                + "</workingHours></workingTimeAttendantPeriods><extension>100</extension><attendantAliases>operator 0</attendantAliases><enableLiveAttendant>true</enableLiveAttendant></Rule>"
                + "<Rule><id>%s</id><name>Voicemail</name><enabled>true</enabled><type>Internal</type><description>Default voicemail dialing plan</description><permissions/><gatewayAware>false</gatewayAware>"
                + "<authorizationChecked>true</authorizationChecked><internal>true</internal><mediaServerType>freeswitchMediaServer</mediaServerType><pstnPrefixOptional>false</pstnPrefixOptional>"
                + "<longDistancePrefixOptional>false</longDistancePrefixOptional><externalLen>0</externalLen><afterHoursAttendantEnabled>false</afterHoursAttendantEnabled>"
                + "<enableLiveAttendant>false</enableLiveAttendant></Rule></Rules>",
            rules.get(0).getId(), rules.get(1).getId(), rules.get(2).getId(), rules.get(3).getId(), rules.get(4).getId(), rules.get(5).getId(), rules.get(6).getId(),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[0].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[0].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[1].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[1].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[2].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[2].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[3].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[3].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[4].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[4].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[5].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[5].getStop()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[6].getStart()),
            dateFormat.format(((AttendantRule)rules.get(6)).getWorkingTimeAttendant().getWorkingHours()[6].getStop()),
            rules.get(7).getId()),
            rulesXml);
        // create attendant dialing rule
        String createRule = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Rule><name>Attendant-2</name><enabled>false</enabled><type>Attendant</type><description>desc</description><permissions/>"
            + "<gatewayAware>false</gatewayAware><authorizationChecked>true</authorizationChecked><internal>true</internal>"
            + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefixOptional>false</longDistancePrefixOptional>"
            + "<externalLen>0</externalLen><afterHoursAttendant>Operator</afterHoursAttendant>"
            + "<afterHoursAttendantEnabled>true</afterHoursAttendantEnabled>"
            + "<holidayAttendant>Operator</holidayAttendant><workingTimeAttendant>Operator</workingTimeAttendant>"
            + "<holidayAttendantPeriods>"
            + "<holidayPeriods><startDate>2014-08-18T00:00:00+03:00</startDate><endDate>2014-08-19T00:00:00+03:00</endDate></holidayPeriods>"
            + "<holidayPeriods><startDate>2014-08-18T00:00:00+03:00</startDate><endDate>2014-08-19T00:00:00+03:00</endDate></holidayPeriods>"
            + "</holidayAttendantPeriods><workingTimeAttendantPeriods><workingHours><enabled>false</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>MONDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>TUESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>WEDNESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>FRIDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>false</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>SATURDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>SUNDAY</scheduledDay></workingHours></workingTimeAttendantPeriods>"
            + "<extension>1211</extension><attendantAliases>attoper 5</attendantAliases><did>123</did><enableLiveAttendant>true</enableLiveAttendant></Rule>";
        int code = postXmlString(createRule, "rules");
        assertEquals(200, code);

        int savedRuleId = db().queryForInt("select dialing_rule_id from dialing_rule where name='Attendant-2'");

        //retrieve saved rule
        String ruleXml = getAsXml(String.format("rules/%s", savedRuleId));
        assertEquals(
                String.format("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                    + "<Rule><id>%s</id><name>Attendant-2</name><enabled>false</enabled><type>Attendant</type>"
                    + "<description>desc</description><permissions/><gatewayAware>false</gatewayAware>"
                    + "<authorizationChecked>true</authorizationChecked><internal>true</internal>"
                    + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefixOptional>false</longDistancePrefixOptional>"
                    + "<externalLen>0</externalLen><afterHoursAttendant>Operator</afterHoursAttendant>"
                    + "<afterHoursAttendantEnabled>true</afterHoursAttendantEnabled>"
                    + "<holidayAttendant>Operator</holidayAttendant>"
                    + "<workingTimeAttendant>Operator</workingTimeAttendant>"
                    + "<holidayAttendantPeriods/><workingTimeAttendantPeriods>"
                    + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
                    + "<stop>1970-01-01T20:00:00+02:00</stop>"
                    + "<scheduledDay>MONDAY</scheduledDay></workingHours>"
                    + "<workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                    + "<scheduledDay>TUESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
                    + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                    + "<scheduledDay>WEDNESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
                    + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                    + "<scheduledDay>THURSDAY</scheduledDay></workingHours><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
                    + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>FRIDAY</scheduledDay></workingHours>"
                    + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                    + "<scheduledDay>SATURDAY</scheduledDay></workingHours><workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
                    + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>SUNDAY</scheduledDay></workingHours></workingTimeAttendantPeriods>"
                    + "<extension>1211</extension><attendantAliases>attoper 5</attendantAliases><did>123</did><enableLiveAttendant>true</enableLiveAttendant></Rule>",
                        savedRuleId), ruleXml);
        // modify rule
        String modifyRule = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Rule><name>Attendant-2-modified</name><enabled>false</enabled><type>Attendant</type><description>desc</description><permissions/>"
            + "<gatewayAware>false</gatewayAware><authorizationChecked>true</authorizationChecked><internal>true</internal>"
            + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefixOptional>false</longDistancePrefixOptional>"
            + "<externalLen>0</externalLen><afterHoursAttendant>Operator</afterHoursAttendant>"
            + "<afterHoursAttendantEnabled>true</afterHoursAttendantEnabled>"
            + "<holidayAttendant>Operator</holidayAttendant><workingTimeAttendant>Operator</workingTimeAttendant>"
            + "<holidayAttendantPeriods>"
            + "<holidayPeriods><startDate>2014-08-18T00:00:00+03:00</startDate><endDate>2014-08-19T00:00:00+03:00</endDate></holidayPeriods>"
            + "</holidayAttendantPeriods><workingTimeAttendantPeriods><workingHours><enabled>false</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>MONDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>TUESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>WEDNESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>FRIDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>false</enabled>"
            + "<start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop>"
            + "<scheduledDay>SATURDAY</scheduledDay></workingHours>"
            + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
            + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>SUNDAY</scheduledDay></workingHours></workingTimeAttendantPeriods>"
            + "<extension>1211</extension><attendantAliases>attoper 5</attendantAliases><did>123</did><enableLiveAttendant>true</enableLiveAttendant></Rule>";
        int putCode = putXmlString(modifyRule, String.format("rules/%s", savedRuleId));
        assertEquals(200, putCode);

        //retrieve modified rule
        int count = db().queryForInt("select count(*) from dialing_rule where name='Attendant-2-modified'");
        assertEquals(1, count);
        ruleXml = getAsXml(String.format("rules/%s", savedRuleId));
        assertEquals(
                String.format(
                    modifyRule = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                        + "<Rule><id>%s</id><name>Attendant-2-modified</name><enabled>false</enabled><type>Attendant</type><description>desc</description><permissions/>"
                        + "<gatewayAware>false</gatewayAware><authorizationChecked>true</authorizationChecked><internal>true</internal>"
                        + "<pstnPrefixOptional>false</pstnPrefixOptional><longDistancePrefixOptional>false</longDistancePrefixOptional>"
                        + "<externalLen>0</externalLen><afterHoursAttendant>Operator</afterHoursAttendant>"
                        + "<afterHoursAttendantEnabled>true</afterHoursAttendantEnabled>"
                        + "<holidayAttendant>Operator</holidayAttendant><workingTimeAttendant>Operator</workingTimeAttendant>"
                        + "<holidayAttendantPeriods/>"
                        + "<workingTimeAttendantPeriods><workingHours><enabled>false</enabled>"
                        + "<start>1970-01-01T11:00:00+02:00</start>"
                        + "<stop>1970-01-01T20:00:00+02:00</stop>"
                        + "<scheduledDay>MONDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
                        + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                        + "<scheduledDay>TUESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled>"
                        + "<start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                        + "<scheduledDay>WEDNESDAY</scheduledDay></workingHours><workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start>"
                        + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>THURSDAY</scheduledDay></workingHours>"
                        + "<workingHours><enabled>true</enabled><start>1970-01-01T11:00:00+02:00</start><stop>1970-01-01T20:00:00+02:00</stop>"
                        + "<scheduledDay>FRIDAY</scheduledDay></workingHours>"
                        + "<workingHours><enabled>false</enabled>"
                        + "<start>1970-01-01T11:00:00+02:00</start>"
                        + "<stop>1970-01-01T20:00:00+02:00</stop>"
                        + "<scheduledDay>SATURDAY</scheduledDay></workingHours>"
                        + "<workingHours><enabled>false</enabled><start>1970-01-01T11:00:00+02:00</start>"
                        + "<stop>1970-01-01T20:00:00+02:00</stop><scheduledDay>SUNDAY</scheduledDay></workingHours></workingTimeAttendantPeriods>"
                        + "<extension>1211</extension><attendantAliases>attoper 5</attendantAliases><did>123</did><enableLiveAttendant>true</enableLiveAttendant></Rule>",
                        savedRuleId), ruleXml);
        //delete rule
        int deleteRule = delete(String.format("rules/%s", savedRuleId));
        assertEquals(200, deleteRule);
        count = db().queryForInt("select count(*) from dialing_rule where name='Attedant-2-modified'");
        assertEquals(0, count);
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setDialPlanSetup(DialPlanSetup dialPlanSetup) {
        m_dialPlanSetup = dialPlanSetup;
    }
}
