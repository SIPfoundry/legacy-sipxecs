/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.reginfo;

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
import org.sipfoundry.reginfo.Reginfo;

/**
 * Unit test for simple App.
 */
public class AppTest  extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public AppTest(String testName) {
        super(testName);
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite() {
        return new TestSuite(AppTest.class);
    }

    /**
     * Rigourous Test :-)
     */
    public void testApp() {
		try {

			// note that you can use multiple bindings with the same class, in
			// which case you need to use the getFactory() call that takes the
			// binding name as the first parameter
			IBindingFactory bfact = BindingDirectory.getFactory(Reginfo.class);

			/*
			 * // unmarshal customer information from file IUnmarshallingContext uctx = bfact.createUnmarshallingContext();
			 * FileInputStream in = new FileInputStream(args[0]); DialogInfo dialogInfo = (DialogInfo)uctx.unmarshalDocument(in,
			 * null); // you can add code here to alter the unmarshalled customer
			 */

			Reginfo reginfo = new Reginfo();
			reginfo.setVersion(1L);
			reginfo.setState("partial");

			// marshal object back out to file (with nice indentation, as UTF-8)
			IMarshallingContext mctx = bfact.createMarshallingContext();
			mctx.setIndent(0, "\r\n", ' ');
			FileOutputStream out = new FileOutputStream("out.xml");
			mctx.marshalDocument(reginfo, "UTF-8", null, out);

			 // unmarshal reginfo from file
			IUnmarshallingContext uctx = bfact.createUnmarshallingContext();
			FileInputStream in = new FileInputStream("out.xml");
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
