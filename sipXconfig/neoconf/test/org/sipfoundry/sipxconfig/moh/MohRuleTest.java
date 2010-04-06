/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.moh;

import java.io.StringWriter;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

public class MohRuleTest extends XMLTestCase {
    MohRule m_mohRule;

    @Override
    protected void setUp() throws Exception {
        m_mohRule = new MohRule("example.org", "mohUser");
    }

    public void testGetPattern() {
        String[] patterns = m_mohRule.getPatterns();
        assertTrue(patterns.length == 1);
        assertEquals("mohUser.", patterns[0]);
    }

    public void testGetCallTag() {
        assertEquals(CallTag.MOH, m_mohRule.getCallTag());
    }

    public void testGetTransforms() throws Exception {
        Document doc = DocumentFactory.getInstance().createDocument();
        Element element = doc.addElement("test");

        Transform[] transforms = m_mohRule.getTransforms();
        assertTrue(transforms.length == 1);
        transforms[0].addToParent(element);
        StringWriter xml = new StringWriter();
        doc.write(xml);
        assertXMLEqual("<test><transform><user>IVR</user><host>example.org</host>"
                + "<urlparams>action=moh</urlparams><urlparams>moh=u{vdigits}</urlparams>"
                + "</transform></test>", xml.toString());
    }
}
