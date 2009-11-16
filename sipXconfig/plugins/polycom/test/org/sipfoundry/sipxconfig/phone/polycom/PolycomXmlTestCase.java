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

import org.custommonkey.xmlunit.DetailedDiff;
import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;

public abstract class PolycomXmlTestCase extends XMLTestCase {

    public PolycomXmlTestCase() {

        super();

        XMLUnit.setIgnoreComments(true);
        XMLUnit.setIgnoreAttributeOrder(true);
    }

    protected void assertPolycomXmlEquals(java.io.Reader control, java.io.Reader test)
        throws org.xml.sax.SAXException, java.io.IOException {

        System.out.println("*** BEGIN actual profile content. ***");
        int ch;
        try {
            do {
                ch = test.read();
              if (ch != -1) {
                System.out.print((char) ch);
            }
            } while (ch != -1);

        } catch (IOException e) {
            e.printStackTrace();
        }
        test.reset();
        System.out.println("*** END actual profile content. ***");

        Diff phoneDiff = new DetailedDiff(new Diff(control, test));
        assertXMLEqual(phoneDiff, true);
    }
}
