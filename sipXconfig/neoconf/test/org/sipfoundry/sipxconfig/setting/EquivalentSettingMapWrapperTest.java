/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class EquivalentSettingMapWrapperTest extends TestCase {

    private EquivalentSettingMapWrapper uniqueMap1, uniqueMap2, uniqueMap3;

    public void setUp() {
        Setting animalDog = new SettingImpl("animal");
        animalDog.setValue("dog");
        animalDog.setIndex(0);

        Setting animalDogDupe = new SettingImpl("animal");
        animalDogDupe.setValue("dog");
        animalDogDupe.setIndex(1);

        Setting animalCat = new SettingImpl("animal");
        animalCat.setValue("cat");
        animalCat.setIndex(2);

        SettingMap map1 = new SettingMap();
        map1.put("animal", animalDog);
        uniqueMap1 = new EquivalentSettingMapWrapper(map1);

        SettingMap map2 = new SettingMap();
        map2.put("animal", animalCat);
        uniqueMap2 = new EquivalentSettingMapWrapper(map2);

        SettingMap map3 = new SettingMap();
        map3.put("animal", animalDogDupe);
        uniqueMap3 = new EquivalentSettingMapWrapper(map3);
    }

    public void testIdentity() {
        assertEquals(uniqueMap1, uniqueMap1);
    }

    public void testEquivalence() {
        assertEquals(uniqueMap1, uniqueMap3);
        assertEquals(uniqueMap3, uniqueMap1);
        assertEquals(uniqueMap1.hashCode(), uniqueMap3.hashCode());

        assertFalse(uniqueMap1.equals(uniqueMap2));
        assertFalse(uniqueMap2.equals(uniqueMap1));
    }

}
