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

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Collection of speeddial buttons associated with the user.
 */
public class SpeedDial extends BeanWithId {
    private User m_user;

    private List<Button> m_buttons = new ArrayList<Button>();

    public List<Button> getButtons() {
        return m_buttons;
    }

    public void setButtons(List<Button> buttons) {
        m_buttons = buttons;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public void replaceButtons(List<Button> buttons) {
        m_buttons.clear();
        m_buttons.addAll(buttons);
    }

    public void moveButtons(int index, int moveOffset) {
        List<Button> buttons = getButtons();
        DataCollectionUtil.move(buttons, index, moveOffset);
    }

    /**
     * Returns the URL of the resource list coresponfing to this speed dial.
     * 
     * @param consolidated needs to be set to true for phones that are not RFC compliant and
     *        expect "consolidated" view of resource list (see XCF-1453)
     */
    public String getResourceListId(boolean consolidated) {
        StringBuilder listId = new StringBuilder("~~rl~");
        listId.append(getUser().getId());
        if (consolidated) {
            listId.append('c');
        }
        return listId.toString();
    }

    public String getResourceListName() {
        return getUser().getUserName();
    }

    /**
     * If at least one button supports BLF we need to register this list as BLF
     */
    public boolean isBlf() {
        for (Button button : getButtons()) {
            if (button.isBlf()) {
                return true;
            }
        }
        return false;
    }
}
