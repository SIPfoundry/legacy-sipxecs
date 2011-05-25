/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig;

import java.io.InputStream;
import java.io.Reader;
import java.io.Writer;
import java.util.Iterator;
import java.util.Map;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;

import junit.framework.Assert;

import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.QName;
import org.dom4j.io.DOMWriter;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.test.TestUtil;

/**
 * Collection of utility function to XMLUnit and DOM4J testing with XMLUnit
 */
public abstract class XmlUnitHelper {
    /**
     * In most cases where we use the xpath it's easier to ignore namespace than to construct
     * proper namespace aware XPatch expression
     *
     * @param namespaceAware
     */
    public static void setNamespaceAware(boolean namespaceAware) {
        DocumentBuilderFactory testDocumentBuilderFactory = XMLUnit
                .getTestDocumentBuilderFactory();
        testDocumentBuilderFactory.setNamespaceAware(namespaceAware);

        DocumentBuilderFactory controlDocumentBuilderFactory = XMLUnit
                .getControlDocumentBuilderFactory();
        controlDocumentBuilderFactory.setNamespaceAware(namespaceAware);
    }

    /**
     * Dumps DOM4J document to Strings.
     *
     * @param doc DOM4J document
     * @return String containing XML document
     * @deprecated Use {@link TestUtil#asString(Document)} instead
     */
    public static String asString(Document doc) {
        return TestUtil.asString(doc);
    }

    /**
     * Dumps DOM4J document to standard error.
     *
     * @param doc
     */
    public static void dumpXml(Document doc) {
        System.err.println(TestUtil.asString(doc));
    }

    /**
     * Asserts that the element in expected namespace URI
     *
     * @param element dom4jj element
     * @param expectedNamespaceUri URI of the namespace
     */
    public static void assertElementInNamespace(Element element, String expectedNamespaceUri) {
        QName name = element.getQName();
        String namespaceURI = name.getNamespaceURI();
        Assert.assertEquals(expectedNamespaceUri, namespaceURI);
    }

    public static org.w3c.dom.Document getDomDoc(Document doc) throws Exception {
        DOMWriter writer = new DOMWriter();
        return writer.write(doc);
    }

    public static void style(Reader xsl, Reader xml, Writer out, Map params)
            throws TransformerException {
        Source xmlSource = new javax.xml.transform.stream.StreamSource(xml);
        TransformerFactory factory = TransformerFactory.newInstance();
        Source xslSource = new javax.xml.transform.stream.StreamSource(xsl);
        Transformer transformer;
        transformer = factory.newTransformer(xslSource);
        if (params != null && !params.isEmpty()) {
            Iterator entries = params.entrySet().iterator();
            while (entries.hasNext()) {
                Map.Entry entry = (Map.Entry) entries.next();
                transformer.setParameter((String) entry.getKey(), entry.getValue());
            }
        }
        StreamResult result = new StreamResult(out);
        transformer.transform(xmlSource, result);
    }

    /**
     * Loads XML document from class resource
     *
     * @param klass - for locating the file - pass this.class
     * @param name name of the file in the same directory as klass
     * @return newly read DOM4J document
     */
    public static Document loadDocument(Class klass, String name) throws DocumentException {
        InputStream stream = klass.getResourceAsStream(name);
        SAXReader reader = new SAXReader();
        return reader.read(stream);
    }
}
