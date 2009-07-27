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

import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class SpeedDialGroupTest extends TestCase {

    public void testGetSpeedDial() {
        List<Button> buttons = new ArrayList<Button>();
        buttons.add(new Button("test_button_one", "1000"));
        buttons.add(new Button("test_button_two", "1001"));
        buttons.add(new Button("test_button_three", "1002"));

        SpeedDialGroup speedDialGroup = new SpeedDialGroup();
        speedDialGroup.setButtons(buttons);

        User user = new User();
        SpeedDial speedDial = speedDialGroup.getSpeedDial(user);
        List<Button> speedDialButtons = speedDial.getButtons();

        assertEquals(buttons.size(), speedDialButtons.size());
        for (int i = 0; i <  buttons.size(); i++) {
            assertEquals(buttons.get(i).getLabel(), speedDialButtons.get(i).getLabel());
            assertEquals(buttons.get(i).getNumber(), speedDialButtons.get(i).getNumber());
        }

        // Make sure deleting the speedDial buttons does not affect speedDialGroup buttons
        for (int i = 0; i <  speedDialButtons.size(); i++) {
            speedDialButtons.remove(i);
        }

        assertEquals(buttons.size(), speedDialGroup.getButtons().size());
    }
}
