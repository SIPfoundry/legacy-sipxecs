/*
 *
 *
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ipdialog;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.ipdialog.IpDialogPhone.IpDialogContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class IpDialogPhoneTest extends TestCase {

    private final Collection<PhonebookEntry> m_emptyPhonebook = Collections.<PhonebookEntry> emptyList();

    public void testGetFileName() throws Exception {

        IpDialogPhone phone = new IpDialogPhone();
        phone.setSerialNumber("123456789123");
        assertEquals("SipTone/config/ipdSIP123456789123.xml", phone.getProfileFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {

        IpDialogPhone phone = new IpDialogPhone();
        PhoneModel model = new PhoneModel("ipDialog");
        model.setLabel("ipDialog SipTone V");
        model.setModelDir("ipDialog");
        model.setProfileTemplate("ipDialog/ipDialog.vm");
        phone.setModel(model);
        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.generateProfiles(location);
        InputStream expectedProfile = getClass().getResourceAsStream("expected-config");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, location.toString());
    }

    public void testGenerateProfilesForSpeedDial() throws Exception {

        IpDialogPhone phone = new IpDialogPhone();
        PhoneModel model = new PhoneModel("ipDialog");
        model.setLabel("ipDialog SipTone V");
        model.setModelDir("ipDialog");
        model.setProfileTemplate("ipDialog/ipDialog.vm");
        phone.setModel(model);

        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        Button[] buttons = new Button[] {
            new Button("abc", "123@sipfoundry.org"), new Button("def_def", "456"), new Button("xyz abc", "789"),
            new Button("ABC", "147"), new Button("Joe User", "258@sipfoundry.com"), new Button("qwe", "369")
        };

        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();
        phoneContextControl.replay();

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);
        String profile = location.toString();

        assertTrue(profile.contains("<memKeys>"));
        assertTrue(profile.contains("<mem1>abc^123@sipfoundry.org^1^0^0</mem1>"));
        assertTrue(profile.contains("<mem2>def_def^456^1^0^0</mem2>"));
        assertTrue(profile.contains("<mem3>xyz abc^789^1^0^0</mem3>"));
        assertTrue(profile.contains("<mem4>ABC^147^1^0^0</mem4>"));
        assertTrue(profile.contains("<mem5>Joe User^258@sipfoundry.com^1^0^0</mem5>"));
        assertTrue(profile.contains("<mem6>qwe^369^1^0^0</mem6>"));

        assertFalse(profile.contains("<mem7></mem7>"));
        assertFalse(profile.contains("<mem8></mem8>"));
        assertFalse(profile.contains("<mem9></mem9>"));
        assertFalse(profile.contains("<mem10></mem10>"));
        assertFalse(profile.contains("<mem11></mem11>"));
        assertFalse(profile.contains("<mem12></mem12>"));
        assertFalse(profile.contains("<mem13></mem13>"));
        assertFalse(profile.contains("<mem14></mem14>"));
        assertFalse(profile.contains("<mem15></mem15>"));
        assertFalse(profile.contains("<mem16></mem16>"));
        assertTrue(profile.contains("</memKeys>"));

        phoneContextControl.verify();
    }

    public void testIpDialogContextEmpty() {

        IpDialogPhone phone = new IpDialogPhone();

        IpDialogContext sc = new IpDialogPhone.IpDialogContext(phone, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDialNumber");

        assertEquals(0, numbers.length);
    }

    public void testIpDialogTotalNumSpeedDials() {

        SpeedDial smallSd = createSpeedDial(5);
        SpeedDial largeSd = createSpeedDial(17);

        IpDialogPhone phone = new IpDialogPhone();

        IpDialogContext sc = new IpDialogPhone.IpDialogContext(phone, smallSd, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDialNumber");
        assertEquals(5, numbers.length);
        for (int i = 0; i < numbers.length; i++) {
            assertEquals(Integer.toString(i), numbers[i]);
        }

        sc = new IpDialogPhone.IpDialogContext(phone, largeSd, m_emptyPhonebook, null);
        numbers = (String[]) sc.getContext().get("speedDialNumber");
        assertEquals(16, numbers.length);
        for (int i = 0; i < numbers.length; i++) {
            assertEquals(Integer.toString(i), numbers[i]);
        }
    }

    private SpeedDial createSpeedDial(int size) {

        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < size; i++) {
            buttons.add(new Button("test", Integer.toString(i)));
        }
        SpeedDial sd = new SpeedDial();
        sd.setButtons(buttons);
        return sd;
    }

}
