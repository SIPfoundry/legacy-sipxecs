/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class CiscoAtaProfileWriterTest extends TestCase {
    private static final char LF = 0x0a;

    public void testWriteEntry() {
        StringWriter wtr = new StringWriter();
        CiscoAtaProfileWriter pwtr = new CiscoAtaProfileWriter();
        pwtr.setWriter(wtr);
        pwtr.writeEntry("bird", "crow");
        assertEquals("bird:crow" + LF, wtr.toString());
    }

    public void testWriteBitmap() {
        SettingSet birds = new SettingSet();
        SettingImpl bird = new SettingImpl(".BM.Bird");
        birds.addSetting(bird);
        SettingImpl bird0 = new SettingImpl(".BF.0.Bird");
        bird0.setValue("256");
        birds.addSetting(bird0);

        StringWriter wtr = new StringWriter();
        CiscoAtaProfileWriter pwtr = new CiscoAtaProfileWriter();
        pwtr.setWriter(wtr);
        birds.acceptVisitor(pwtr);

        assertEquals("Bird:0x100" + LF, wtr.toString());
    }

    public void testWriteProxy() {
        LineInfo info = new LineInfo();
        info.setRegistrationServer("example.org");
        info.setRegistrationServerPort("5060");

        CiscoAtaProfileWriter pwtr = new CiscoAtaProfileWriter();

        StringWriter wtr = new StringWriter();
        pwtr.setWriter(wtr);
        pwtr.writeProxy(info, true);
        assertEquals("Proxy:example.org" + LF, wtr.toString());

        wtr = new StringWriter();
        pwtr.setWriter(wtr);
        pwtr.writeProxy(info, false);
        assertEquals("Proxy:example.org:5060" + LF, wtr.toString());


        info.setRegistrationServerPort("5062");
        wtr = new StringWriter();
        pwtr.setWriter(wtr);
        pwtr.writeProxy(info, true);
        assertEquals("Proxy:example.org:5062" + LF, wtr.toString());

        info.setRegistrationServerPort("5062");
        wtr = new StringWriter();
        pwtr.setWriter(wtr);
        pwtr.writeProxy(info, false);
        assertEquals("Proxy:example.org:5062" + LF, wtr.toString());
    }
}
