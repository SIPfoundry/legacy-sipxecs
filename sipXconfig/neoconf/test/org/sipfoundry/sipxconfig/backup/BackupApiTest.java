package org.sipfoundry.sipxconfig.backup;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BackupApiTest {

    @Test
    public void test() throws IOException {
        BackupApi api = new BackupApi();
        BackupPlan plan = new BackupPlan();
        Map<String,String> archiveIds = new HashMap<String, String>();
        archiveIds.put("A", "Alpha");
        archiveIds.put("B", "Beta");
        archiveIds.put("C", "Gamma");
        plan.setType(BackupType.local);
        plan.setLimitedCount(10);
        DailyBackupSchedule s1 = new DailyBackupSchedule();
        s1.setBackupPlan(plan);
        s1.setScheduledDay(ScheduledDay.FRIDAY);
        s1.setTimeOfDay(new TimeOfDay(1, 2));    
        plan.setEncodedDefinitionString("A,BEE,C");
        plan.setSchedules(Arrays.asList(s1));
        StringWriter actual = new StringWriter();
        BackupSettings settings = new BackupSettings();
        settings.setSettings(TestHelper.loadSettings("backup/backup.xml"));
        List<String> backups = Arrays.asList("one", "two", "three");
        api.writeBackup(actual, plan, backups, settings, archiveIds);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected.json"));
        TestHelper.assertEqualJson2(expected, actual.toString());
    }
}
