package org.sipfoundry.sipxconfig.region;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class RegionApiTest {

    @Test
    public void write() throws IOException {
        List<Region> rs = Arrays.asList(new Region("r1"), new Region("r2"));
        rs.get(0).setAddresses(new String[]{"1.1.1.1", "1.1.2.0/24"});
        String expected = IOUtils.toString(getClass().getResourceAsStream("regions.expected.json"));        
        TestHelper.assertEquals(expected, rs);
    }

    @Test
    public void read() throws IOException {
        String json = "{\"name\":\"Foo\",\"addresses\":[\"2.2.2.2\",\"2.2.2.0/24\"],\"id\":-1},\"id\":1}";
        ObjectMapper mapper = new ObjectMapper();
        Region r = mapper.readValue(json, new TypeReference<Region>() { });
        assertEquals("Foo", r.getName());
        assertEquals(2, r.getAddresses().length);
    }
}
