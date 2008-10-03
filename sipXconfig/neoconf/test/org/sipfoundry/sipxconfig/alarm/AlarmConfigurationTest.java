package org.sipfoundry.sipxconfig.alarm;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmConfiguration;

public class AlarmConfigurationTest extends XMLTestCase {
    @Override
    protected void setUp() throws Exception {

    }

    public void testGenerateAlarms() throws Exception {
        AlarmConfiguration alarmConf = new AlarmConfiguration();
        alarmConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmConf.setTemplate("alarms/sipXalarms-config.vm");
        List<Alarm> alarms = new ArrayList<Alarm>();

        Alarm alarm1 = new Alarm();
        alarm1.setAlarmId("LOGIN_FAILED");
        alarm1.setCode("SPX00005");
        alarm1.setSeverity("warning");
        alarm1.setComponent("sipXconfig");
        alarm1.setEmailEnabled(true);
        alarm1.setMinThreshold(3);

        Alarm alarm2 = new Alarm();
        alarm2.setAlarmId("PROCESS_FAILED");
        alarm2.setCode("SPX00004");
        alarm2.setSeverity("crit");
        alarm2.setComponent("sipXsupervisor");
        alarm2.setEmailEnabled(false);

        alarms.add(alarm1);
        alarms.add(alarm2);

        alarmConf.generate(alarms);

        String generatedXml = alarmConf.getFileContent();

        InputStream referenceXmlStream = AlarmConfigurationTest.class
                .getResourceAsStream("alarms-config-test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));

    }
}
