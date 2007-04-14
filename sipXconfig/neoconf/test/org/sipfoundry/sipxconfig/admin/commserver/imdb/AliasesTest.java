/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class AliasesTest extends XMLTestCase {
    private final static String[][] DATA = {
        {
            "301@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }, {
            "302@example.org", "\"Jane Doe\"<sip:jane.doe@example.org>;q=0.5"
        }, {
            "302@example.org", "\"Betty Boop\"<sip:betty.boop@example.org>;q=0.8"
        }, {
            "302@example.org", "\"Bill Boop\"<sip:bill.boop@example.org>;q=0.8"
        }, {
            "303@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }
    };

    private Aliases m_aliases;

    public AliasesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    protected void setUp() throws Exception {
        m_aliases = new Aliases();
    }

    public void testAddAliases() throws Exception {
        List aliases = new ArrayList();
        for (int i = 0; i < DATA.length; i++) {
            String[] aliasRow = DATA[i];
            AliasMapping mapping = new AliasMapping();
            mapping.setIdentity(aliasRow[0]);
            mapping.setContact(aliasRow[1]);
            aliases.add(mapping);
        }

        Element element = m_aliases.createItemsElement(DataSet.ALIAS);
        m_aliases.addAliases(element, aliases);

        String aliasesXml = XmlUnitHelper.asString(element.getDocument());

        InputStream referenceXmlStream = AliasesTest.class.getResourceAsStream("alias.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(aliasesXml));
    }
}
