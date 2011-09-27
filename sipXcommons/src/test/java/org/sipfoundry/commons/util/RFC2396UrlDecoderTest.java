/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.util;

import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class RFC2396UrlDecoderTest extends TestCase {

   public RFC2396UrlDecoderTest(String testName) {
       super(testName);
   }

   public void testDecode() {
       String url = null;
       assertEquals(url = "a@example.com", RFC2396UrlDecoder.decode(url));
       assertEquals(url = "a@example.com%", RFC2396UrlDecoder.decode(url));
       assertEquals(url = "a@example.com%3", RFC2396UrlDecoder.decode(url));
       assertEquals("a@example.com0", RFC2396UrlDecoder.decode("a@example.com%30"));
   }
   
   public void testSampling() throws IOException {
       InputStream urisIn = RFC2396UrlDecoderTest.class.getResourceAsStream("sample-uris.txt");
       List uris = IOUtils.readLines(urisIn);
       for (Object uri : uris) {
           RFC2396UrlDecoder.decode((String)uri);
       }
       IOUtils.closeQuietly(urisIn);
   }
}
