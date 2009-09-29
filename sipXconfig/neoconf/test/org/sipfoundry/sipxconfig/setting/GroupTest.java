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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.BeanWithGroupsTest.BirdWithGroups;

public class GroupTest extends TestCase {

    private SettingSet m_root;

    private SettingImpl m_pea;

    protected void setUp() {
        m_root = new SettingSet();
        SettingSet fruit = (SettingSet) m_root.addSetting(new SettingSet("fruit"));
        fruit.addSetting(new SettingImpl("apple"));
        SettingSet vegetable = (SettingSet) m_root.addSetting(new SettingSet("vegetable"));
        m_pea = (SettingImpl) vegetable.addSetting(new SettingImpl("pea"));
        m_pea.setValue("snowpea");
    }

    public void testCompare() {
        assertEquals(0, new Group().compareTo(new Group()));

        Group g1 = new Group();
        g1.setName("a");
        Group g2 = new Group();
        g2.setName("b");
        assertTrue(g1.compareTo(g2) < 0);

        g1.setWeight(new Integer(5));
        g2.setWeight(new Integer(3));
        assertTrue(g1.compareTo(g2) > 0);
    }

    public void testInherhitSettingsForEditing() {
        BirdWithGroups bird = new BirdWithGroups();
        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        settings.getSetting("towhee/canyon").setValue("12");

        // bird bean should remain untouched
        assertNull(bird.getSettingValue("towhee/canyon"));

        assertEquals("12", settings.getSetting("towhee/canyon").getValue());
        assertEquals(1, g.size());
    }

    public void testInherhitSettingsForEditingIgnoreMatchingDefaults() {
        BirdWithGroups bird = new BirdWithGroups();
        Setting originalSetting = bird.getSettings().getSetting("pigeon/passenger");
        assertEquals("0", originalSetting.getValue());

        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        Setting inheritedSetting = settings.getSetting("pigeon/passenger");

        inheritedSetting.setValue("1");

        assertEquals(1, g.size());
        assertEquals("1", inheritedSetting.getValue());
        assertEquals("0", inheritedSetting.getDefaultValue());
        assertEquals("0", originalSetting.getValue());
        assertEquals("0", originalSetting.getDefaultValue());

        bird.addGroup(g);
        // inherited setting did not change even after we added bird to the group
        assertEquals("1", inheritedSetting.getValue());
        assertEquals("0", inheritedSetting.getDefaultValue());

        // original setting now different - since we are in the group
        assertEquals("1", originalSetting.getValue());
        assertEquals("1", originalSetting.getDefaultValue());

        inheritedSetting.setValue("0");
        assertEquals(0, g.size());
        assertEquals("0", inheritedSetting.getValue());
        assertEquals("0", inheritedSetting.getDefaultValue());
        assertEquals("0", originalSetting.getValue());
        assertEquals("0", originalSetting.getDefaultValue());
    }

    public void testInherhitSettingsForEditingGetDefaultValue() {
        BirdWithGroups bird = new BirdWithGroups();
        assertEquals("0", bird.getSettingValue("woodpecker/ivory-billed"));

        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        Setting ivoryBilled = settings.getSetting("woodpecker/ivory-billed");

        ivoryBilled.setValue("2");

        assertEquals("0", ivoryBilled.getDefaultValue());
        assertEquals("2", ivoryBilled.getValue());
    }

    public void testSelectGroupWithHighestWeight() {
        Group firstGroup = new Group();
        firstGroup.setWeight(10);
        Group secondGroup = new Group();
        secondGroup.setWeight(15);
        Group thirdGroup = new Group();
        thirdGroup.setWeight(5);

        List<Group> groups = new ArrayList<Group>();
        Collections.<Group>addAll(groups, firstGroup, secondGroup, thirdGroup);

        Group heaviestGroup = Group.selectGroupWithHighestWeight(groups);
        assertEquals(firstGroup, heaviestGroup);
    }
}
