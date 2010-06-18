/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.speeddial;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialGroup;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SpeedDialPanel extends BaseComponent {

    @Parameter
    public abstract SpeedDial getSpeedDial();

    public abstract void setSpeedDial(SpeedDial speedDial);

    @Parameter
    public abstract SpeedDialGroup getSpeedDialGroup();

    @Parameter(required = false, defaultValue = "false")
    public abstract boolean isAddNumberLinkDisabled();

    @Parameter(required = false, defaultValue = "false")
    public abstract boolean isSubscribeToPresenceDisabled();

    public abstract void setSpeedDialGroup(SpeedDialGroup speedDialGroup);

    public abstract List<Button> getButtons();

    public abstract void setButtons(List<Button> buttons);

    public abstract Button getButton();

    public abstract int getIndex();

    public abstract boolean getAdd();

    public abstract int getRemoveIndex();

    public abstract void setRemoveIndex(int index);

    public abstract int getMoveIndex();

    public abstract void setMoveIndex(int index);

    public abstract int getMoveOffset();

    public abstract void setMoveOffset(int offset);

    public int getButtonsSize() {
        return getButtons().size();
    }

    public void setButtonsSize(int size) {
        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < size; i++) {
            buttons.add(new Button());
        }
        setButtons(buttons);
    }

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setRemoveIndex(-1);
        setMoveIndex(-1);
        setMoveOffset(0);
        if (!TapestryUtils.isRewinding(cycle, this)) {
            if (null != getSpeedDial()) {
                setButtons(getSpeedDial().getButtons());
            } else if (null != getSpeedDialGroup()) {
                setButtons(getSpeedDialGroup().getButtons());
            }
        }
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            afterRewind(cycle);
        }
    }

    private void afterRewind(IRequestCycle cycle) {
        List<Button> buttons = getButtons();
        if (TapestryUtils.isValid(cycle, this) && getAdd()) {
            buttons.add(new Button());
        }
        int removeIndex = getRemoveIndex();
        if (removeIndex >= 0) {
            buttons.remove(removeIndex);
        }
        if (null != getSpeedDial()) {
            SpeedDial speedDial = getSpeedDial();
            speedDial.replaceButtons(buttons);
            int moveIndex = getMoveIndex();
            if (moveIndex >= 0) {
                speedDial.moveButtons(moveIndex, getMoveOffset());
            }
        } else if (null != getSpeedDialGroup()) {
            SpeedDialGroup speedDialGroup = getSpeedDialGroup();
            speedDialGroup.replaceButtons(buttons);
            int moveIndex = getMoveIndex();
            if (moveIndex >= 0) {
                speedDialGroup.moveButtons(moveIndex, getMoveOffset());
            }
        }
    }
}
