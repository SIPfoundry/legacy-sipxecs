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

import java.io.StringWriter;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.XmlUnitHelper;

/**
 * FullTransformTest
 */
public class TransformTest extends XMLTestCase {
    private Element m_element;
    private Document m_doc;

    protected void setUp() throws Exception {
        m_doc = DocumentFactory.getInstance().createDocument();
        m_element = m_doc.addElement("test");
    }

    public void testEmptyFullTransform() throws Exception {
        Transform transform = new FullTransform();
        transform.addToParent(m_element);
        StringWriter xml = new StringWriter();
        m_doc.write(xml);
        assertXMLEqual("<test><transform/></test>", xml.toString());
    }

    public void testEmptyUrlTransform() throws Exception {
        Transform transform = new UrlTransform();
        transform.addToParent(m_element);
        StringWriter xml = new StringWriter();
        m_doc.write(xml);
        assertXMLEqual("<test><transform/></test>", xml.toString());
    }

    public void testUrlTransform() throws Exception {
        UrlTransform transform = new UrlTransform();
        String url = "<sip:12343@host.domain>";
        transform.setUrl(url);
        transform.addToParent(m_element);
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(m_doc);
        assertXpathEvaluatesTo(url, "test/transform/url", domDoc);
    }

    public void testFullTransform() throws Exception {
        FullTransform transform = new FullTransform();
        String host = "10.1.1.4";
        String user = "911";
        String[] fieldParams = {
            "q=1.0"
        };
        String[] headerParams = {
            "h1", "h2"
        };
        String[] urlParams = {
            "u1", "u2", "u3"
        };

        transform.setHost(host);
        transform.setUser(user);
        transform.setFieldParams(fieldParams);
        transform.setHeaderParams(headerParams);
        transform.setUrlParams(urlParams);

        transform.addToParent(m_element);

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(m_doc);

        assertXpathEvaluatesTo(host, "test/transform/host", domDoc);
        assertXpathEvaluatesTo(user, "test/transform/user", domDoc);
        assertXpathEvaluatesTo(fieldParams[0], "test/transform/fieldparams", domDoc);
        assertXpathEvaluatesTo(headerParams[1], "test/transform/headerparams[2]", domDoc);
        assertXpathEvaluatesTo(urlParams[0], "test/transform/urlparams", domDoc);
        assertXpathEvaluatesTo(urlParams[1], "test/transform/urlparams[2]", domDoc);
        assertXpathEvaluatesTo(urlParams[2], "test/transform/urlparams[3]", domDoc);
    }
}
