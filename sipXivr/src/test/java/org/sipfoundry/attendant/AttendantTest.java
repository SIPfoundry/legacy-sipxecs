/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.attendant;

import java.util.Hashtable;
import java.util.ListResourceBundle;
import java.util.Vector;

import org.sipfoundry.sipxivr.Configuration;
import org.sipfoundry.sipxivr.FreeSwitchEvent;
import org.sipfoundry.sipxivr.TextToPrompts_en;
import org.sipfoundry.sipxivr.FreeSwitchEventSocketEmulator;
import org.sipfoundry.sipxivr.ValidUsersXML;

import junit.framework.TestCase;

public class AttendantTest extends TestCase {
    public class MyResources extends ListResourceBundle {
        private final Object[][] m_contents = {
            {
                "global.prefix", "glob/"
            }, {
                "goodbye.prompts", "goodbye"
            },
        };

        public Object[][] getContents() {
            return m_contents;
        }

    }

    /*
     * public void testAttendant() { fail("Not yet implemented"); }
     * 
     * public void testGetPromptList() { fail("Not yet implemented"); }
     * 
     * public void testGetPlayer() { fail("Not yet implemented"); }
     * 
     * public void testRun() { fail("Not yet implemented"); }
     * 
     * public void testAttendant1() { fail("Not yet implemented"); }
     * 
     * public void testDoAction() { fail("Not yet implemented"); }
     * 
     * public void testTransfer() { fail("Not yet implemented"); }
     * 
     * public void testGetRecordedName() { fail("Not yet implemented"); }
     * 
     * public void testSelectChoice() { fail("Not yet implemented"); }
     * 
     * public void testDialByName() { fail("Not yet implemented"); }
     */

    public void testGoodbye() {
        FreeSwitchEventSocketEmulator fses = new FreeSwitchEventSocketEmulator();
        Hashtable<String, String> params = new Hashtable<String, String>();
        Configuration ivrConfig = Configuration.update(false);

        Attendant a = new Attendant(ivrConfig, fses, params);
        a.setAttendantBundle(new MyResources());
        a.setTtp(new TextToPrompts_en());
        a.setAttendantConfig(org.sipfoundry.attendant.Configuration.update(false));
        a.setValidUsers(ValidUsersXML.update(false));
        a.setSchedules(new Schedule());

        try {
            Vector<String> response = new Vector<String>();
            response.add("Content-Type: text/event-plain");
            fses.appendDtmfQueue("4");
            fses.appendDtmfQueue("2");
            fses.event = new FreeSwitchEvent(response, "event-name: CHANNEL_EXECUTE_COMPLETE\n");
            a.goodbye();
        } catch (Throwable e) {
            e.printStackTrace();
            fail("Uh Oh.");
        }

        Vector<String> cmds = new Vector<String>();
        for (String string : fses.cmds.elementAt(0).split("\n")) {
            cmds.add(string);
        }
        assertEquals("glob/goodbye", FreeSwitchEvent.parseHeaders(cmds).get("execute-app-arg"));

        cmds.clear();
        for (String string : fses.cmds.elementAt(1).split("\n")) {
            cmds.add(string);
        }
        assertEquals("hangup", FreeSwitchEvent.parseHeaders(cmds).get("execute-app-name"));
    }

    /*
     public void testFailure() {
     fail("Not yet implemented");
     }
     */
}
