/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.test;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringReader;
import java.util.List;

import org.junit.Test;


public class SqlFileReaderTest {
    
    @Test
    public void parse() throws IOException {
        StringReader seed = new StringReader("a\nb;\nc;\n--ignore");
        SqlFileReader rdr = new SqlFileReader(seed);
        List<String> actual = rdr.parse();
        assertArrayEquals(new String[] {"a b", "c"}, actual.toArray(new String[0]));
    }
    
    @Test
    public void parseSample() throws IOException {
        SqlFileReader rdr = new SqlFileReader(getClass().getResourceAsStream("sample.sql"));
        List<String> actual = rdr.parse();            
        assertEquals(5, actual.size());
    }    
}
