package org.sipfoundry.sipxconfig.rest;

import java.io.StringWriter;
import java.text.DateFormat;
import java.util.Date;
import java.util.Locale;

import org.restlet.data.MediaType;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;

import junit.framework.TestCase;

public class ConfigServerTimeResourceTest extends TestCase {

    public void testRepresentDataTxt() throws Exception {
        ConfigServerTimeResource resource = new ConfigServerTimeResource();

        Representation representation = resource.represent(new Variant(MediaType.TEXT_PLAIN));
        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();

        DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.DEFAULT,
                DateFormat.SHORT, Locale.getDefault());

        Date date = dateFormat.parse(generated);
        assertTrue(date.getTime() <= new Date().getTime());
    }
}
