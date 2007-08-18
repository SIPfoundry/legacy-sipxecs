/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.custommonkey.xmlunit.XMLAssert;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AutoAttendantTest extends XMLTestCase {

    private VxmlGenerator m_vxml;

    protected void setUp() throws Exception {
        super.setUp();
        XMLUnit.setIgnoreWhitespace(true);

        m_vxml = new VxmlGenerator();
        String etc = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        m_vxml.setScriptsDirectory(etc);
        m_vxml.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    public void testGetSystemName() {
        AutoAttendant aa = new AutoAttendant();
        assertEquals("xcf-1", aa.getSystemName());
        assertFalse(aa.isAfterhour());
        assertFalse(aa.isOperator());
        assertFalse(aa.isPermanent());

        AutoAttendant operator = new AutoAttendant();
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        assertEquals("operator", operator.getSystemName());
        assertFalse(operator.isAfterhour());
        assertTrue(operator.isOperator());
        assertTrue(operator.isPermanent());

        AutoAttendant afterhour = new AutoAttendant();
        afterhour.setSystemId(AutoAttendant.AFTERHOUR_ID);
        assertEquals("afterhour", afterhour.getSystemName());
        assertTrue(afterhour.isAfterhour());
        assertFalse(afterhour.isOperator());
        assertTrue(afterhour.isPermanent());
    }
    
    private AutoAttendant createAutoAttendant() {
        return new AutoAttendant() {
            protected Setting loadSettings() {
                return TestHelper.loadSettings("sipxvxml/autoattendant.xml");
            }
        };        
    }

    // TODO: fix the test after autoattendant.vm has been changed
    // see: http://paradise.pingtel.com/viewsvn/sipX?view=rev&rev=6846
    // test should not depend on real autoattendant.vm
    public void testActivateDefaultAttendant() throws Exception {
        AutoAttendant aa = createAutoAttendant();
        aa.setVxmlGenerator(new VxmlGenerator() {
            public String getPromptsDirectory() {
                return "prompts/";
            }
        });
        aa.setPrompt("prompt.wav");
        aa.addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
        aa.addMenuItem(DialPad.NUM_1, new AttendantMenuItem(AttendantMenuAction.DISCONNECT));
        aa.setSettingValue("onfail/transfer", "1");
        aa.setSettingValue("onfail/transfer-extension", "999");

        StringWriter actualXml = new StringWriter();
        m_vxml.generate(aa, actualXml);
        assertVxmlEquals("expected-autoattendant.xml", actualXml.toString());
    }
    
    private void assertVxmlEquals(String expectedFile, String actualXml) throws Exception {
        Reader actualRdr = new StringReader(actualXml);
        StringWriter actual = new StringWriter();
        XmlUnitHelper.style(getReader("autoattendant.xsl"), actualRdr, actual, null);

        Reader expectedRdr = getReader(expectedFile);
        StringWriter expected = new StringWriter();
        XmlUnitHelper.style(getReader("autoattendant.xsl"), expectedRdr, expected, null);

        XMLAssert.assertXMLEqual(expected.toString(), actual.toString());
    }

    private Reader getReader(String resource) {
        InputStream stream = getClass().getResourceAsStream(resource);
        return new InputStreamReader(stream);
    }
}
