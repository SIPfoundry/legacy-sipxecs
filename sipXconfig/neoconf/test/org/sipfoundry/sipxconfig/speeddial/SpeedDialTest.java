/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

import junit.framework.TestCase;

public class SpeedDialTest extends TestCase {

    public void testGetResourceListId() {
        User user = new User() {
            @Override
            public Integer getId() {
                return 115;
            }

            @Override
            public String getUserName() {
                return "test_user";
            }
        };

        SpeedDial sd = new SpeedDial();
        sd.setUser(user);
        assertEquals("~~rl~F~test_user", sd.getResourceListId(false));
        assertEquals("~~rl~C~test_user", sd.getResourceListId(true));
    }

    public void testSpeedDialMaxNumber() {
        SpeedDial sd = new SpeedDial();
        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < 136; i++) {
            Button b = new Button();
            b.setBlf(true);
            b.setNumber("123");
            buttons.add(b);
        }
        sd.setButtons(buttons);

        AliasManager aliasManager = EasyMock.createMock(AliasManager.class);
        aliasManager.isAliasInUse((String) EasyMock.anyObject());
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        EasyMock.replay(aliasManager);
        SpeedDialManagerImpl impl = new SpeedDialManagerImpl();
        impl.setAliasManager(aliasManager);
        boolean npeCaught = false;
        try {
            impl.saveSpeedDial(sd);
        } catch (NullPointerException expected) {
            npeCaught = true;
        }
        assertTrue(npeCaught);
    }

    public void testSpeedDialExceedsMaxNumber() {
        SpeedDial sd = new SpeedDial();
        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < 137; i++) {
            Button b = new Button();
            b.setBlf(true);
            b.setNumber("123");
            buttons.add(b);
        }
        sd.setButtons(buttons);

        AliasManager aliasManager = EasyMock.createMock(AliasManager.class);
        aliasManager.isAliasInUse((String) EasyMock.anyObject());
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        EasyMock.replay(aliasManager);
        SpeedDialManagerImpl impl = new SpeedDialManagerImpl();
        impl.setAliasManager(aliasManager);
        boolean ueCaught = false;
        try {
            impl.saveSpeedDial(sd);
        } catch (UserException expected) {
            ueCaught = true;
        }
        assertTrue(ueCaught);
    }

    public void testSpeedDialMaxNumberWithNonBlf() {
        SpeedDial sd = new SpeedDial();
        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < 136; i++) {
            Button b = new Button();
            b.setBlf(true);
            b.setNumber("123");
            buttons.add(b);
        }
        Button nonBlf = new Button();
        nonBlf.setBlf(false);
        nonBlf.setNumber("123");
        buttons.add(nonBlf);

        sd.setButtons(buttons);

        AliasManager aliasManager = EasyMock.createMock(AliasManager.class);
        aliasManager.isAliasInUse((String) EasyMock.anyObject());
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        EasyMock.replay(aliasManager);
        SpeedDialManagerImpl impl = new SpeedDialManagerImpl();
        impl.setAliasManager(aliasManager);
        boolean ueCaught = false;
        try {
            impl.saveSpeedDial(sd);
        } catch (UserException expected) {
            ueCaught = true;
        }
        assertTrue(ueCaught);
    }
}
