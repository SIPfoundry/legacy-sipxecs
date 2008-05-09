package org.sipfoundry.sipxconfig.setting;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class EquivalentSettingMapWrapperTest {

    private EquivalentSettingMapWrapper uniqueMap1, uniqueMap2, uniqueMap3;

    @Before
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

    @Test
    public void testIdentity() {
        Assert.assertTrue(uniqueMap1.equals(uniqueMap1));
    }

    @Test
    public void testEquivalence() {
        Assert.assertTrue(uniqueMap1.equals(uniqueMap3));
        Assert.assertTrue(uniqueMap3.equals(uniqueMap1));
        Assert.assertFalse(uniqueMap1.equals(uniqueMap2));
        Assert.assertFalse(uniqueMap2.equals(uniqueMap1));
    }

}
