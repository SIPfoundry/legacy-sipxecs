package org.sipfoundry.sipxconfig.dns;


import java.io.IOException;
import java.io.StringWriter;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsCustomApiTest {

    @Test
    public void write() throws IOException {
        String expected = IOUtils.toString(getClass().getResourceAsStream("custom.expected.json"));
        StringWriter actual = new StringWriter();
        DnsCustomApi api = new DnsCustomApi();
        DnsCustomRecords custom = new DnsCustomRecords();
        custom.setName("c");
        custom.setRecords("x");
        api.writeCustom(actual, Collections.singleton(custom));
        TestHelper.assertEqualJson2(expected, actual.toString());
    }
}
