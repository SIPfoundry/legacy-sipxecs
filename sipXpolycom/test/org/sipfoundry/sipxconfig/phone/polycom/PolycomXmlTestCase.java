/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import static org.easymock.EasyMock.createMock;

import java.io.InputStream;
import java.io.PrintStream;
import java.io.Reader;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.custommonkey.xmlunit.DetailedDiff;
import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Attribute;
import org.dom4j.Element;
import org.dom4j.io.DOMReader;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.xml.sax.InputSource;

public abstract class PolycomXmlTestCase extends XMLTestCase {
    private static final String XML_INDENT = "  ";
    static protected PolycomPhone phone40;
    static protected PolycomPhone phone41;
    static protected MemoryProfileLocation location;
    static protected VelocityProfileGenerator m_pg;

    public PolycomXmlTestCase() {

        super();

        XMLUnit.setIgnoreComments(true);
        XMLUnit.setIgnoreAttributeOrder(true);
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    protected void setUp4041Tests() {
        XMLUnit.setIgnoreWhitespace(true);

        PhoneModel model = new PolycomModel();
        model.setMaxLineCount(6);
        model.setSupportedFeatures(PolycomXmlTestCase.supportedVVX500);
        model.setModelId("polycomVVX500");
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);

        phone41 = new PolycomPhone();
        phone41.setModelId("polycomVVX500");
        phone41.setPhoneModelSource(phoneModelSource);
        phone41.setBeanId("polycomVVX500");
        phone41.setModel(model);
        phone41.setDeviceVersion(PolycomModel.VER_4_1_X);
        PhoneTestDriver.supplyTestData(phone41);

        phone40 = new PolycomPhone();
        phone40.setModelId("polycomVVX500");
        phone40.setBeanId("polycomVVX500");
        phone40.setPhoneModelSource(phoneModelSource);
        phone40.setModel(model);
        phone40.setDeviceVersion(PolycomModel.VER_4_0_X);
        PhoneTestDriver.supplyTestData(phone40);
        
        location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }
    
    static Set<String> supportedVVX500 = new HashSet<String>(Arrays.asList(new String[] {
            "disableCallList", "intercom", "voiceQualityMonitoring", "nway-conference", "localConferenceCallHold",
            "singleKeyPressConference", "VVX_500_CodecPref", "desktopIntegration", "exchangeIntegration", "video"
        }));

    protected void assertPolycomXmlEquals(InputStream controlStream, Reader testReader) throws Exception {
        org.w3c.dom.Document controlDoc = getDocument(controlStream);
        org.w3c.dom.Document testDoc = getDocument(testReader);

        Diff phoneDiff = new DetailedDiff(new Diff(controlDoc, testDoc));
        assertXMLEqual(phoneDiff, true);
    }

    protected void assertPolycomXmlEquals(Reader controlReader, Reader testReader) throws Exception {
        Diff phoneDiff = new DetailedDiff(new Diff(controlReader, testReader));
        
        assertXMLEqual(phoneDiff, true);
    }
    
    private org.w3c.dom.Document getDocument(InputStream is) throws Exception {
        DocumentBuilder db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        org.w3c.dom.Document controlDoc = db.parse(is);
        return controlDoc;
    }

    private org.w3c.dom.Document getDocument(Reader reader) throws Exception {
        DocumentBuilder db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        org.w3c.dom.Document controlDoc = db.parse(new InputSource(reader));
        return controlDoc;
    }

    protected void dumpXml(InputStream is, PrintStream out) throws Exception {
        dumpXml(new DOMReader().read(getDocument(is)).getRootElement(), out, "");
    }

    protected void dumpXml(Reader reader, PrintStream out) throws Exception {
        dumpXml(new DOMReader().read(getDocument(reader)).getRootElement(), out, "");
    }

    protected void dumpXml(Element element, PrintStream out, String indent) {
        out.print(indent + "<" + element.getName());
        List<Attribute> attributes = element.attributes();
        for (Attribute attribute : attributes) {
            out.println(indent);
            out.print(XML_INDENT + indent + attribute.getName() + "=\"" + attribute.getText() + "\"");
        }

        // Note: Polycom XML elements have no text, only attributes and child elements.

        List<Element> children = element.elements();
        if (children.isEmpty()) {
            out.println("");
            out.println(indent + "/>");
        } else {
            out.println(">");
            for (Element child : children) {
                out.println(indent);
                dumpXml(child, out, indent + XML_INDENT + XML_INDENT);
            }
            out.println(indent + "</" + element.getName() + ">");
        }
    }

}
