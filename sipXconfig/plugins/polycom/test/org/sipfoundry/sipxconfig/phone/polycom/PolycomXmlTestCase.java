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

import java.io.InputStream;
import java.io.PrintStream;
import java.io.Reader;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.custommonkey.xmlunit.DetailedDiff;
import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Attribute;
import org.dom4j.Element;
import org.dom4j.io.DOMReader;
import org.xml.sax.InputSource;

public abstract class PolycomXmlTestCase extends XMLTestCase {
    private static final String XML_INDENT = "  ";

    public PolycomXmlTestCase() {

        super();

        XMLUnit.setIgnoreComments(true);
        XMLUnit.setIgnoreAttributeOrder(true);
        XMLUnit.setIgnoreWhitespace(true);
    }

    protected void assertPolycomXmlEquals(InputStream controlStream, Reader testReader) throws Exception {
        org.w3c.dom.Document controlDoc = getDocument(controlStream);
        org.w3c.dom.Document testDoc = getDocument(testReader);

        Diff phoneDiff = new DetailedDiff(new Diff(controlDoc, testDoc));
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
