/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.util;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

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
}
