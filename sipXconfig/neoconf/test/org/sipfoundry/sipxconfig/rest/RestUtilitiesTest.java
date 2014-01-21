package org.sipfoundry.sipxconfig.rest;

import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.FileNotFoundException;

import org.junit.Before;
import org.junit.Test;

public class RestUtilitiesTest {

    @Before
    public void setUp() throws Exception {
    }

    @Test
    public void testGetLink() throws FileNotFoundException {
        File f = new File(getClass().getResource("sample.txt").getFile());
        String actual = RestUtilities.getLink(f, 1, "text/plain");
        assertTrue(actual.startsWith("/sipxconfig/download.svc?contentType=text%2Fplain&digest=e3e1c4211b595b96d28d96b58c972d4d&path="));
        assertTrue(actual.endsWith("%2Fsample.txt"));
    }
}
