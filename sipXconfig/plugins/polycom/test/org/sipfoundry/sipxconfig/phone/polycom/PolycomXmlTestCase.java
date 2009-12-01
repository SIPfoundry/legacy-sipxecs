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

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.Reader;
import java.util.List;

import org.custommonkey.xmlunit.DetailedDiff;
import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.dom.DOMDocumentFactory;
import org.dom4j.io.SAXReader;
import org.xml.sax.SAXException;

public abstract class PolycomXmlTestCase extends XMLTestCase {

    public PolycomXmlTestCase() {

        super();

        XMLUnit.setIgnoreComments(true);
        XMLUnit.setIgnoreAttributeOrder(true);
        XMLUnit.setIgnoreWhitespace(true);
    }

    protected void assertPolycomXmlEquals(InputStream controlStream, Reader testReader) throws SAXException,
            IOException, DocumentException {

        assertPolycomXmlEquals(controlStream, getDocument(testReader));
    }

    protected void assertPolycomXmlEquals(InputStream controlStream, Document testDoc) throws DocumentException {

        assertPolycomXmlEquals(getDocument(controlStream), testDoc);
    }

    @Deprecated
    protected void assertPolycomXmlEquals(Reader controlReader, Reader testReader) throws IOException, SAXException,
            DocumentException {

        assertPolycomXmlEquals(getDocument(controlReader), getDocument(testReader));
    }

    private static final String XML_INDENT = "  ";

    protected void dumpXml(Element element, PrintStream out) {
        dumpXml(element, out, "");
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

    protected void dumpXml(Document document, PrintStream out) {
        dumpXml(document.getRootElement(), out, "");
    }

    protected void dumpXml(InputStream stream, PrintStream out) throws DocumentException {
        dumpXml(getDocument(stream), out);
    }

    protected void dumpXml(Reader reader, PrintStream out) throws DocumentException {
        dumpXml(getDocument(reader), out);
    }

    protected Document getDocument(Reader reader) throws DocumentException {
        return new SAXReader(new DOMDocumentFactory()).read(reader);
    }

    protected Document getDocument(InputStream stream) throws DocumentException {
        return new SAXReader(new DOMDocumentFactory()).read(stream);
    }

    protected void assertPolycomXmlEquals(Document controlDoc, Document testDoc) {

        System.out.println("*** BEGIN actual profile content. ***");
        dumpXml(testDoc, System.out);
        System.out.println("*** END actual profile content. ***");

        Diff phoneDiff = new DetailedDiff(
                new Diff((org.w3c.dom.Document) controlDoc, (org.w3c.dom.Document) testDoc));
        assertXMLEqual(phoneDiff, true);
    }

}
