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

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public class XmlModelBuilderTest extends TestCase {
    private ModelBuilder m_builder;

    protected void setUp() throws Exception {
        m_builder = new XmlModelBuilder("etc");
    }

    @Deprecated
    public void testSettingPropertySettersDeprecated() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "simplemodel.xml");
        SettingSet root = m_builder.buildModel(in);
        Setting group = root.getSetting("group");
        assertEquals("Group Label", group.getLabel());
        assertEquals("Group Description", group.getDescription());

        Setting setting = group.getSetting("setting");
        assertEquals("Setting Label", setting.getLabel());
        assertEquals("Setting Description", setting.getDescription());
    }

    @Deprecated
    public void testCollectionPropertySettersDeprecated() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");
        SettingSet games = m_builder.buildModel(in);
        Setting deck = games.getSetting("cards/deck");
        assertNotNull(deck);

        assertEquals("Collection Label", deck.getLabel());
        assertEquals("Collection Description", deck.getDescription());
    }

    public void testSettingPropertySetters() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "simplemodel.xml");
        SettingSet root = m_builder.buildModel(in);
        Setting group = root.getSetting("group");
        assertEquals("Group Profile Name", group.getProfileName());

        Setting setting = group.getSetting("setting");
        assertEquals("Setting Profile Name", setting.getProfileName());
        assertSame(StringSetting.DEFAULT, setting.getType());
    }

    public void testReadingGames() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");
        SettingSet games = m_builder.buildModel(in);
        assertEquals("", games.getName());
        assertEquals(2, games.getValues().size());

        SettingSet chess = (SettingSet) games.getSetting("chess");
        assertEquals(chess.getName(), "chess");
        assertEquals("The game of chess", chess.getLabel());
        assertEquals(2, chess.getValues().size());
        Iterator orderPreserved = chess.getValues().iterator();
        SettingSet colors = (SettingSet) orderPreserved.next();
        assertEquals(2, colors.getValues().size());
        SettingSet pieces = (SettingSet) orderPreserved.next();
        assertEquals(6, pieces.getValues().size());

        Setting pawn = pieces.getSetting("pawn");
        SettingType type = pawn.getType();
        assertEquals("enum", type.getName());
        EnumSetting enumType = (EnumSetting) type;
        Collection moves = enumType.getEnums().keySet();
        assertNotNull(moves);
        assertEquals(3, moves.size());
        assertTrue(moves.contains("diagonal one to take another piece"));

        Setting cards = games.getSetting("cards");
        assertTrue(cards instanceof SettingSet);
        assertEquals(1, cards.getValues().size());

        Setting suite = cards.getSetting("deck/suit[0]");
        assertNotNull(suite);
        Setting card = cards.getSetting("deck/card[3]");
        assertEquals(3, card.getIndex());
        assertEquals("cards/deck/card[3]", card.getPath());
        assertEquals(13, card.getValues().size());
    }

    /**
     * marginal value, testing a bug...
     */
    public void testIteration() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");
        SettingSet games = m_builder.buildModel(in);

        Iterator i = games.getValues().iterator();
        while (i.hasNext()) {
            assertTrue(SettingSet.class.isAssignableFrom(i.next().getClass()));
        }
    }

    @Deprecated
    public void testInheritanceDeprecated() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "genders.xml");
        SettingSet root = m_builder.buildModel(in);

        Setting human = root.getSetting("human");
        assertEquals("Human", human.getLabel());
        assertEquals("Earthlings", human.getDescription());

        Setting man = root.getSetting("man");
        assertEquals("Man", man.getLabel());
        assertEquals("Earthlings", man.getDescription());

        Setting woman = root.getSetting("woman");
        assertEquals("Woman", woman.getLabel());
        assertEquals("Earthlings", woman.getDescription());
    }

    public void testInheritance() throws IOException {
        File in = TestHelper.getResourceAsFile(getClass(), "genders.xml");
        SettingSet root = m_builder.buildModel(in);

        Setting human = root.getSetting("human");
        assertNotNull(human.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertNull(human.getSetting("giveBirth"));

        Setting man = root.getSetting("man");
        assertNotNull(man.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("face", man.getSetting("shave").getValue());
        assertNull(man.getSetting("giveBirth"));

        Setting woman = root.getSetting("woman");
        assertNotNull(woman.getSetting("eat").getSetting("fruit").getSetting("apple"));
        assertEquals("legs", woman.getSetting("shave").getValue());
        assertNotNull(woman.getSetting("giveBirth"));

        // test for true clones
        man.getSetting("shave").setValue("back");
        assertEquals("back", man.getSetting("shave").getValue());
        assertNotSame("back", woman.getSetting("shave").getValue());
    }

    public void testFlags() throws Exception {
        File in = TestHelper.getResourceAsFile(getClass(), "genders.xml");
        SettingSet root = m_builder.buildModel(in);
        Setting reason = root.getSetting("man/reason");
        assertFalse(reason.isAdvanced());
        assertTrue(reason.isHidden());
        Setting giveBirth = root.getSetting("woman/giveBirth");
        assertTrue(giveBirth.isAdvanced());
        assertFalse(giveBirth.isHidden());
    }

    public void testCollectionFlags() throws Exception {
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");
        SettingSet games = m_builder.buildModel(in);
        Setting deck = games.getSetting("cards/deck");
        assertNotNull(deck);
        assertTrue(deck.isAdvanced());
        assertTrue(deck.isHidden());
    }

    public void testNullValue() throws Exception {
        File in = TestHelper.getResourceAsFile(getClass(), "simplemodel.xml");
        SettingSet root = m_builder.buildModel(in);
        assertNull(root.getSetting("group/setting").getValue());
    }
}
