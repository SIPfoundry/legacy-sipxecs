/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.hivemind.Messages;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.components.EnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryContext;

public abstract class AttendantMenuPanel extends BaseComponent {

    private static final String ACTION_PREFIX = "menuItemAction.";

    @InjectObject(value = "spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract EvenOdd getRowClass();

    @Parameter(required = true)
    public abstract AttendantMenu getMenu();

    @Parameter(defaultValue = "literal:0")
    public abstract String getMinKey();

    @Parameter(defaultValue = "literal:#")
    public abstract String getMaxKey();

    /**
     * If true - the only possible action offered will be forward to extension
     */
    @Parameter(defaultValue = "ognl:false")
    public abstract boolean getExtensionOnly();

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public abstract DialPad getAddMenuItemDialPad();

    public abstract void setAddMenuItemDialPad(DialPad dialPad);

    public abstract AttendantMenuAction getAddMenuItemAction();

    public abstract void setAddMenuItemAction(AttendantMenuAction action);

    public abstract void setMenuItems(AttendantMenuItemMapAdapter menuItems);

    public abstract AttendantMenuItemMapAdapter getMenuItems();

    public DialPadSelectionModel getDialPadSelectionModel() {
        return new DialPadSelectionModel(getMinKey(), getMaxKey());
    }

    public IPropertySelectionModel getLocalizedMenuItemActions() {
        Messages messages = getMessages();
        return getTapestry().instructUserToSelect(new ActionSelectionModel(messages), messages);
    }

    public IPropertySelectionModel getAutoAttendants() {
        List<AutoAttendant> autoAttendants = getAutoAttendantManager().getAutoAttendants();
        IPropertySelectionModel attendantSelectionModel = new AttendantSelectionModel(
                autoAttendants);
        return getTapestry().instructUserToSelect(attendantSelectionModel, getMessages());
    }

    public static class DialPadSelectionModel extends EnumPropertySelectionModel {
        public DialPadSelectionModel(String min, String max) {
            DialPad minKey = DialPad.getByName(min);
            DialPad maxKey = DialPad.getByName(max);
            DialPad[] options = DialPad.getRange(minKey, maxKey);
            setOptions(options);
        }
    }

    public static class ActionSelectionModel extends LocalizedOptionModelDecorator {
        public ActionSelectionModel(Messages messages) {
            EnumPropertySelectionModel model = new EnumPropertySelectionModel();
            model.setEnumClass(AttendantMenuAction.class);
            setModel(model);
            setResourcePrefix(ACTION_PREFIX);
            setMessages(messages);
        }
    }

    public static class AttendantSelectionModel extends ObjectSelectionModel {
        public AttendantSelectionModel(Collection attendants) {
            setCollection(attendants);
            setLabelExpression("name");
            setValueExpression("systemName");

        }
    }

    public String getColumns() {
        return getExtensionOnly() ? "* !dialpad,!extension" : "* !dialpad,!action,!parameter";
    }

    public void addMenuItem() {
        AttendantMenuAction action = getExtensionOnly() ? AttendantMenuAction.TRANSFER_OUT
                : getAddMenuItemAction();
        if (action != null) {
            getMenu().addMenuItem(getAddMenuItemDialPad(), action);
            selectNextAvailableDialpadKey();
            setAddMenuItemAction(null);
        }
    }

    /**
     * Try to select the next likely dial pad key
     */
    private void selectNextAvailableDialpadKey() {
        DialPad minKey = DialPad.getByName(getMinKey());
        DialPad maxKey = DialPad.getByName(getMaxKey());
        DialPad nextKey = getMenu().getNextKey(minKey, maxKey);
        setAddMenuItemDialPad(nextKey);
    }

    public void removeMenuItems() {
        Collection<String> selected = getSelections().getAllSelected();
        getMenu().removeMenuItems(selected);
    }

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);

        selectNextAvailableDialpadKey();
        setMenuItems(new AttendantMenuItemMapAdapter(getMenu()));
    }

    public String getActionName(AttendantMenuAction action) {
        return getMessages().getMessage(ACTION_PREFIX + action.getName());
    }

    /**
     * Let's you set keys on map entries without losing your place in the iteration thru the map.
     * This is handy in tapestry when you iterate thru a list and your key values can change.
     */
    public static class AttendantMenuItemMapAdapter {

        private final Map<DialPad, AttendantMenuItem> m_menuItems;

        private final DialPad[] m_dialPadKeys;

        private DialPad m_currentDialPadKey;

        AttendantMenuItemMapAdapter(AttendantMenu menu) {
            m_menuItems = menu.getMenuItems();
            m_dialPadKeys = m_menuItems.keySet().toArray(new DialPad[m_menuItems.size()]);
        }

        public void setCurrentDialPadKey(DialPad dialPadKey) {
            m_currentDialPadKey = dialPadKey;
        }

        public DialPad getCurrentDialPadKey() {
            return m_currentDialPadKey;
        }

        public DialPad[] getDialPadKeys() {
            return m_dialPadKeys;
        }

        public void setCurrentMenuItemDialPadKeyAssignment(DialPad dialPadKey) {
            AttendantMenuItem value = m_menuItems.get(m_currentDialPadKey);
            m_menuItems.remove(dialPadKey);
            m_menuItems.put(dialPadKey, value);
            m_currentDialPadKey = dialPadKey;
        }

        public DialPad getCurrentMenuItemDialPadKeyAssignment() {
            return m_currentDialPadKey;
        }

        public AttendantMenuItem getCurrentMenuItem() {
            AttendantMenuItem value = m_menuItems.get(m_currentDialPadKey);
            return value;
        }

        public void setCurrentMenuItem(AttendantMenuItem menuItem) {
            m_menuItems.put(m_currentDialPadKey, menuItem);
        }
    }
}
