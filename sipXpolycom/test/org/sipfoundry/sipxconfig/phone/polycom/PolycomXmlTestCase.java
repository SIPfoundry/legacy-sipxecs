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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.custommonkey.xmlunit.DetailedDiff;
import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.XPath;
import org.dom4j.io.DOMReader;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.test.XmlUnitHelper;
import org.xml.sax.InputSource;

public abstract class PolycomXmlTestCase extends XMLTestCase {
    private static final String XML_INDENT = "  ";
    static protected PolycomPhone phone40;
    static protected PolycomPhone phone41;
    static protected PolycomPhone phone50;
    static protected MemoryProfileLocation location;
    static protected VelocityProfileGenerator m_pg;

    public PolycomXmlTestCase() {

        super();

        XMLUnit.setIgnoreComments(true);
        XMLUnit.setIgnoreAttributeOrder(true);
        XMLUnit.setIgnoreWhitespace(true);
    }

    protected void setUp404150Tests() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);

        PolycomModel model = phoneModelBuilder("polycomVVX500", getClass());
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

        phone50 = new PolycomPhone();

        phone50.setModelId("polycomVVX500");
        phone50.setBeanId("polycomVVX500");
        phone50.setPhoneModelSource(phoneModelSource);
        phone50.setModel(model);
        phone50.setDeviceVersion(PolycomModel.VER_5_0_0);
        PhoneTestDriver.supplyTestData(phone50);
        
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

    @SuppressWarnings("unchecked")
    private static List<Element> getModelBeanPropertyElements(Node model_bean, String name) {
        // converting to xsd made xpath foo/ not work, had to switch to */
        String xpath = String.format("*[@name=\"%s\"]/*", name);
        return ((Element) model_bean.selectSingleNode(xpath)).elements();
    }

    private static String getModelBeanPropertyValue(Node model_bean, String name) {
        // converting to xsd made xpath foo/ not work, had to switch to */
        String xpath = String.format("*[@name=\"%s\"]/@value", name);
        return model_bean.selectSingleNode(xpath).getStringValue();
    }

    private static String getModelBeanRefLocalValue(Node model_bean, String name) {
        // converting to xsd made xpath foo/ not work, had to switch to */
        Map<String, String> namespaceUris = new HashMap<String, String>();
        namespaceUris.put("beans", "http://www.springframework.org/schema/beans");

        String xpath = String.format("*[@name=\"%s\"]/beans:ref", name);

        XPath xPath = DocumentHelper.createXPath(xpath);
        xPath.setNamespaceURIs(namespaceUris);

        Node node = xPath.selectSingleNode(model_bean);
        Element el = (Element) node;
        return el.attributeValue("local");
    }

    /**
     * Builds a PolycomModel bean for the specified Model ID.
     * 
     * This method is not specific to Codec Options testing. It could be used generally for other
     * PolycomPhone testing. (Though are definitely some bean properties that are not being
     * populated.)
     * 
     * @throws DocumentException
     */
    public static PolycomModel phoneModelBuilder(String phoneModelId, Class klass) throws Exception {

        PolycomModel model = new PolycomModel();
        model.setModelId(phoneModelId);

        Document beans_document = XmlUnitHelper.loadDocument(klass, "/sipxplugin.beans.xml");

        // Find the bean whose ID matches the specified phone model.
        // converting to xsd made xpath foo/ not work, had to switch to */
        Node model_bean = beans_document.selectSingleNode(String.format("/beans/*[@id=\"%s\"]", phoneModelId));
        assertNotNull(String.format("Failed to find a bean with ID '%s'.", phoneModelId), model_bean);

        // Set the properties.
        model.setLabel(getModelBeanPropertyValue(model_bean, "label"));
        model.setMaxLineCount(Integer.parseInt(getModelBeanPropertyValue(model_bean, "maxLineCount")));

        Set<String> features = new HashSet<String>();
        for (Element supportedFeature : getModelBeanPropertyElements(model_bean, "supportedFeatures")) {
            features.add(supportedFeature.getStringValue());
        }
        model.setSupportedFeatures(features);

        String defaultVersion = getModelBeanRefLocalValue(model_bean, "defaultVersion").substring(57).replace("_", ".");

        for (int i = 0; i < PolycomModel.SUPPORTED_VERSIONS.length; i++) {
            if (PolycomModel.SUPPORTED_VERSIONS[i].getVersionId().equals(defaultVersion)) {
                model.setDefaultVersion(PolycomModel.SUPPORTED_VERSIONS[i]);
                break;
            }
        }



        return model;
    }

}
