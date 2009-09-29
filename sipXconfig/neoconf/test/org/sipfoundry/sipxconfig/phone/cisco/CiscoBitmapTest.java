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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.cisco.CiscoAtaProfileWriter.Bitmap;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;

public class CiscoBitmapTest extends TestCase {
    private SettingImpl m_grossbeak;
    private SettingImpl m_migrating;
    private SettingImpl m_wingbars;
    private Bitmap m_bitmap;

    protected void setUp() {
        m_grossbeak = new SettingImpl(".BM.Grossbeak");
        m_grossbeak.setType(new IntegerSetting());
        m_migrating = new SettingImpl(".BF.0.Grossbeak");
        m_migrating.setType(new BooleanSetting());
        m_wingbars = new SettingImpl(".BF.1.Grossbeak");
        m_wingbars.setType(new IntegerSetting());
        m_bitmap = new Bitmap(m_grossbeak);
    }

    public void testBooleanFalseDefaultFormat() {
        m_bitmap.setBitField(m_migrating);
        assertEquals("0x0", m_bitmap.getProfileValue());
    }

    public void testBooleanTrueFormat() {
        m_migrating.setTypedValue(Boolean.TRUE);
        m_bitmap.setBitField(m_migrating);
        assertEquals("0x1", m_bitmap.getProfileValue());
    }

    public void testIntegerFormat() {
        m_wingbars.setTypedValue(new Integer(2));
        m_bitmap.setBitField(m_wingbars);
        assertEquals("0x4", m_bitmap.getProfileValue());
    }

    public void testGetName() {
        Bitmap bitmap = new Bitmap(m_grossbeak);
        assertEquals("Grossbeak", bitmap.getProfileName());
    }

    public void testSetBitField() {
        Bitmap bitmap = new Bitmap(m_grossbeak);
        bitmap.setBitField(m_migrating);
        assertEquals("0x0", bitmap.getProfileValue());

        m_migrating.setTypedValue(Boolean.TRUE);
        bitmap.setBitField(m_migrating);
        assertEquals("0x1", bitmap.getProfileValue());
    }
}
