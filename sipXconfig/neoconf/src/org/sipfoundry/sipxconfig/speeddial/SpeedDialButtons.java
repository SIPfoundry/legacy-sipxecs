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

public class SpeedDialButtons  extends BeanWithId {

    private List<Button> m_buttons = new ArrayList<Button>();

    public List<Button> getButtons() {
        return m_buttons;
    }

    public void setButtons(List<Button> buttons) {
        m_buttons = buttons;
    }

    public void replaceButtons(List<Button> buttons) {
        m_buttons.clear();
        m_buttons.addAll(buttons);
    }

    public void moveButtons(int index, int moveOffset) {
        List<Button> buttons = getButtons();
        DataCollectionUtil.move(buttons, index, moveOffset);
    }
}
