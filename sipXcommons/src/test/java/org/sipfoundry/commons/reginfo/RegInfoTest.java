/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import org.jibx.runtime.BindingDirectory;
import org.jibx.runtime.IBindingFactory;
import org.jibx.runtime.IMarshallingContext;
import org.jibx.runtime.IUnmarshallingContext;
import org.jibx.runtime.JiBXException;

import org.sipfoundry.commons.reginfo.Reginfo;

/**
 * Unit test for simple App.
 */
public class RegInfoTest  extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public RegInfoTest(String testName) {
        super(testName);
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite() {
        return new TestSuite(RegInfoTest.class);
    }

    /**
     * Rigorous Test :-)
     */
    public void testApp() {
		try {

			// note that you can use multiple bindings with the same class, in
			// which case you need to use the getFactory() call that takes the
			// binding name as the first parameter
			IBindingFactory bfact = BindingDirectory.getFactory(Reginfo.class);

			Reginfo reginfo = new Reginfo();
			reginfo.setVersion(1L);
			reginfo.setState(ReginfoState.PARTIAL);

			Contact contact = new Contact();
			contact.setEvent(ContactEvent.toEnum("Created"));
			System.out.println("Contact.event: " + contact.getEvent().toString());

			// marshal object back out to file (with nice indentation, as UTF-8)
			IMarshallingContext mctx = bfact.createMarshallingContext();
			mctx.setIndent(0, "\r\n", ' ');
			FileOutputStream out = new FileOutputStream("target/out.xml");
			mctx.marshalDocument(reginfo, "UTF-8", null, out);

			 // unmarshal reginfo from file
			IUnmarshallingContext uctx = bfact.createUnmarshallingContext();
			FileInputStream in = new FileInputStream("target/out.xml");
			Reginfo unmarshaledReginfo = (Reginfo)uctx.unmarshalDocument(in, null);
			assertTrue(reginfo.getState().compareTo(unmarshaledReginfo.getState()) == 0);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (JiBXException e) {
			e.printStackTrace();
		}

        assertTrue(true);
    }
}
