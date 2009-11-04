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

    public void assertPolycomXmlEquals(java.io.Reader control, java.io.Reader test)
        throws org.xml.sax.SAXException, java.io.IOException {

        Diff phoneDiff = new DetailedDiff(new Diff(control, test));
        assertXMLEqual(phoneDiff, true);
    }


}
