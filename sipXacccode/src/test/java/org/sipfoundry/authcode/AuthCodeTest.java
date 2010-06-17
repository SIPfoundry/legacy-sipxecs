/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.authcode;

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
import org.sipfoundry.sipxacccode.AccCodeConfiguration;

import junit.framework.TestCase;

public class AuthCodeTest extends TestCase {
    public class MyResources extends ListResourceBundle {
        private final Object[][] m_contents = {
            {
                "global.prefix", "/glob/"
            }, {
                "AuthCode_error_hang_up.prompts", "goodbye"
            },
        };

        public Object[][] getContents() {
            return m_contents;
        }

    }

    public void testGoodbye() throws Exception {
        Hashtable<String, String> params = new Hashtable<String, String>();
        AccCodeConfiguration accCodeConfig = AccCodeConfiguration.getTest();
        FreeSwitchEventSocketEmulator fses = new FreeSwitchEventSocketEmulator(accCodeConfig);

        AuthCode a = new AuthCode(accCodeConfig, fses, params);
        HashMap<Locale, ResourceBundle> resourcesByLocale = new HashMap<Locale, ResourceBundle>();
        resourcesByLocale.put(Locale.ENGLISH, new MyResources());
        Localization loc = new Localization("dog", Locale.ENGLISH.toString(), resourcesByLocale, accCodeConfig, fses);
        a.setLocalization(loc);
        a.setConfig(org.sipfoundry.authcode.Configuration.update(false));

        try {
            Vector<String> response = new Vector<String>();
            response.add("Content-Type: text/event-plain");
            fses.appendDtmfQueue("1234#");
            fses.appendDtmfQueue("66202#");
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
