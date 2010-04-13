/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

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

import org.sipfoundry.commons.dialoginfo.DialogInfo;
import org.sipfoundry.commons.dialoginfo.Participant;
import org.sipfoundry.commons.dialoginfo.ParticipantTarget;
import org.sipfoundry.commons.dialoginfo.ParticipantTargetParam;
import org.sipfoundry.commons.dialoginfo.State;

/**
 * Unit test for simple App.
 */
public class DialogInfoTest  extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public DialogInfoTest(String testName) {
        super(testName);
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite() {
        return new TestSuite(DialogInfoTest.class);
    }

    /**
     * Rigourous Test :-)
     */
    public void testApp() {
		try {
			// note that you can use multiple bindings with the same class, in
			// which case you need to use the getFactory() call that takes the
			// binding name as the first parameter
			IBindingFactory bfact = BindingDirectory.getFactory(DialogInfo.class);

			DialogInfo dialogInfo = new DialogInfo();
			dialogInfo.setVersion(1L);
			dialogInfo.setState(DialogInfoState.PARTIAL);
			dialogInfo.setEntity("sip:201@acme.com");

			org.sipfoundry.commons.dialoginfo.Dialog dialog = new org.sipfoundry.commons.dialoginfo.Dialog();
			dialog.setDirection(DialogDirection.INITIATOR);
			dialog.setId("id12345678");
			dialogInfo.addDialog(dialog);

			State state = new State(DialogState.TRYING);
			dialog.setState(state);

			dialog.addRouteSetHop("hop1");
			dialog.addRouteSetHop("hop2");

			Participant local = new Participant();
			dialog.setLocal(local);

			ParticipantTarget target = new ParticipantTarget();
			target.setUri("sip:201@acme.com");
			local.setTarget(target);

			ParticipantTargetParam param = new ParticipantTargetParam("x-line-id", "1");
			target.addParam(param);

			// marshal object back out to file (with nice indentation, as UTF-8)
			IMarshallingContext mctx = bfact.createMarshallingContext();
			mctx.setIndent(0, "\r\n", ' ');
			FileOutputStream out = new FileOutputStream("target/out.xml");
			mctx.marshalDocument(dialogInfo, "UTF-8", null, out);

			 // unmarshal dialog-info from file
			IUnmarshallingContext uctx = bfact.createUnmarshallingContext();
			FileInputStream in = new FileInputStream("target/out.xml");
			DialogInfo unmarshaledDialogInfo = (DialogInfo)uctx.unmarshalDocument(in, null);
			assertTrue(dialogInfo.getEntity().compareTo(unmarshaledDialogInfo.getEntity()) == 0);

			Dialog unmarshaledDialog = unmarshaledDialogInfo.getDialog(0);
			assertTrue(unmarshaledDialog.getState().get() == DialogState.TRYING);


		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (JiBXException e) {
			e.printStackTrace();
		}

        assertTrue(true);
    }
}
