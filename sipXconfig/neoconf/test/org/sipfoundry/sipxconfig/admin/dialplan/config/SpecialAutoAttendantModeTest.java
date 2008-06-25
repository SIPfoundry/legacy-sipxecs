/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class SpecialAutoAttendantModeTest extends XMLTestCase {

    public SpecialAutoAttendantModeTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDocumentDisabled() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setSystemId("abc");

        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode();
        file.generate(false, aa);
        String expected = "<organizationprefs><specialoperation>false</specialoperation><autoattendant>abc</autoattendant></organizationprefs>";

        assertXMLEqual(expected, file.getFileContent());
    }

    public void testGetDocument() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setSystemId("afterhours");

        InputStream referenceXmlStream = getClass().getResourceAsStream(
                "organizationprefs.test.xml");

        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode();
        file.generate(true, aa);        
        String generatedXml = file.getFileContent();
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }
}
