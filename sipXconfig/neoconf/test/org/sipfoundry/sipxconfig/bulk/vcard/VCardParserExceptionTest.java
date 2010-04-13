/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.vcard;

import junit.framework.TestCase;

public class VCardParserExceptionTest extends TestCase {

    public void testVCardParserException() {
        Exception ex = new VCardParserException();
        assertEquals("&msg.phonebookVcardUploadError", ex.getMessage());
    }
}
