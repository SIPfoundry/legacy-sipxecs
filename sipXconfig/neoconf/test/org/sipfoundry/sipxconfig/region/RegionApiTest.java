package org.sipfoundry.sipxconfig.region;

import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;
import org.junit.Test;

public class RegionApiTest {

    @Test
    public void write() throws IOException {
        List<Region> rs = Arrays.asList(new Region("r1"), new Region("r2"));
        StringWriter actual = new StringWriter();
        new ObjectMapper().writeValue(actual, rs);
        assertEquals("[{\"name\":\"r1\",\"id\":-1},{\"name\":\"r2\",\"id\":-1}]", actual.toString());
    }
    
    @Test
    public void read() throws IOException {
        String json = "{\"name\":\"Foo\",\"id\":1}";
        ObjectMapper mapper = new ObjectMapper();
        Region r = mapper.readValue(json, new TypeReference<Region>() { });
        assertEquals("Foo", r.getName());
    }
}
