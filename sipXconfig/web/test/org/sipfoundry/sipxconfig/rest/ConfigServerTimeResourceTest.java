/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
