/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import org.easymock.EasyMock;

public class SettingTest extends TestCase {

    private SettingSet m_root;

    private AbstractSetting m_apple;

    private AbstractSetting m_fruit;

    private void seedSimpleSettingGroup() {
        m_root = new SettingSet();
        m_fruit = (AbstractSetting) m_root.addSetting(new SettingSet("fruit"));
        m_apple = (AbstractSetting) m_fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingSet("vegatables"));
    }

    public void testDefaultValue() {
        seedSimpleSettingGroup();
        assertEquals("fruit/apple", m_apple.getPath());
        assertEquals("fruit.apple.label", m_apple.getLabelKey());
        assertEquals("fruit.apple.description", m_apple.getDescriptionKey());
        m_apple.setValue("granny smith");
        assertEquals("granny smith", m_apple.getValue());
        assertEquals("granny smith", m_apple.getDefaultValue());
    }

    public void test100GroupsWith100Settings() {
        SettingSet root = new SettingSet();
        for (int i = 0; i < 100; i++) {
            SettingSet model = (SettingSet) root.addSetting(new SettingSet(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                model.addSetting(new SettingImpl(String.valueOf(j)));
            }
            assertEquals(100, model.getValues().size());
        }
        assertEquals(100, root.getValues().size());
    }

    public void testMergeChildren() {
        seedSimpleSettingGroup();
        SettingSet anotherFruit = new SettingSet("fruit");
        Setting banana = new SettingImpl("banana");
        anotherFruit.addSetting(banana);
        m_root.addSetting(anotherFruit);
        assertSame(anotherFruit, m_root.getSetting("fruit"));
        assertSame(banana, anotherFruit.getSetting("banana"));
        assertSame(m_apple, anotherFruit.getSetting("apple"));
    }

    public void testGetProfileName() {
        SettingImpl s = new SettingImpl("bluejay");
        assertEquals("bluejay", s.getProfileName());
        s.setProfileName("indigojay");
        assertEquals("indigojay", s.getProfileName());
    }

    public void testGetProfilePath() {
        SettingImpl birds = new SettingImpl("birds");
        birds.setProfileName("BIRDS");
        AbstractSetting bird = new SettingImpl("bluejay");
        bird.setParent(birds);
        AbstractSetting root = new SettingSet();
        root.addSetting(birds);

        assertEquals("BIRDS", birds.getProfilePath());
        assertEquals("BIRDS/bluejay", bird.getProfilePath());
    }

    public void testGetProfileHandler() {
        SettingImpl s = new SettingImpl("bluejay");
        SettingValue handlerValue = new SettingValueImpl("indigojay");

        SettingModel model = EasyMock.createStrictMock(SettingModel.class);
        model.getProfileName(s);
        EasyMock.expectLastCall().andReturn(handlerValue);
        EasyMock.replay(model);

        s.setModel(model);
        assertSame("indigojay", s.getProfileName());

        EasyMock.verify(model);
    }
}
