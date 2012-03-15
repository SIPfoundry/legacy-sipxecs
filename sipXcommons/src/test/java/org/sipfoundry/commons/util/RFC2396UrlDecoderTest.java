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
