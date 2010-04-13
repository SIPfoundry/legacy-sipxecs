/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.attendant;

import java.util.HashMap;
import java.util.Hashtable;
import java.util.ListResourceBundle;
import java.util.Locale;
import java.util.ResourceBundle;
import java.util.Vector;

import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketEmulator;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.TextToPrompts_en;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrConfiguration;

import junit.framework.TestCase;

public class AttendantTest extends TestCase {
    public class MyResources extends ListResourceBundle {
        private final Object[][] m_contents = {
            {
                "global.prefix", "/glob/"
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

    public void testGoodbye() throws Exception {
        Hashtable<String, String> params = new Hashtable<String, String>();
        IvrConfiguration ivrConfig = IvrConfiguration.getTest();
        FreeSwitchEventSocketEmulator fses = new FreeSwitchEventSocketEmulator(ivrConfig);

        Attendant a = new Attendant(ivrConfig, fses, params);
        HashMap<Locale, ResourceBundle> resourcesByLocale = new HashMap<Locale, ResourceBundle>();
        resourcesByLocale.put(Locale.ENGLISH, new MyResources());
        Localization loc = new Localization("dog", Locale.ENGLISH.toString(), resourcesByLocale, ivrConfig, fses);
        a.setLocalization(loc);
        a.setTtp(new TextToPrompts_en());
        a.setAttendantConfig(org.sipfoundry.attendant.Configuration.update(false));
        a.setValidUsers(ValidUsersXML.update(null, false));
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
        for (String string : fses.cmds.elementAt(1).split("\n")) {
            cmds.add(string);
        }
        assertEquals("/glob/goodbye", FreeSwitchEvent.parseHeaders(cmds).get("execute-app-arg"));

        cmds.clear();
        for (String string : fses.cmds.elementAt(2).split("\n")) {
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
