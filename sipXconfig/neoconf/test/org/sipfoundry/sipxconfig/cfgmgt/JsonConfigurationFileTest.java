package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.junit.Test;

public class JsonConfigurationFileTest {

    @Test
    public void test() throws IOException {
        StringWriter actual = new StringWriter();
        JsonConfigurationFile f = new JsonConfigurationFile(actual);
        f.open("x1");
        actual.write("X1");
        f.close();
        f.open("x2");
        actual.write("X2");
        f.close();
        f.open("y");
        f.open("z1");
        actual.write("Z1");
        f.close();
        f.open("z2");
        actual.write("Z2");
        f.close();
        f.close();
        f.close();
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected.json"));
        assertEquals(expected, actual.toString());
    }
}
