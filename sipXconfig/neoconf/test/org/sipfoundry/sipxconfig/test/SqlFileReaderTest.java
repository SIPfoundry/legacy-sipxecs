/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
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
