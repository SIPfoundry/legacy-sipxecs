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

import java.util.Iterator;
import java.util.Map;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.VxmlGenerator;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAutoAttendant extends PageWithCallback implements
        PageBeginRenderListener {

    public static final String PAGE = "EditAutoAttendant";

    public abstract AutoAttendant getAttendant();

    public abstract void setAttendant(AutoAttendant attendant);

    public abstract VxmlGenerator getVxmlGenerator();

    public abstract SelectMap getSelections();

    public abstract DialPlanContext getDialPlanContext();

    public abstract DialPad getAddMenuItemDialPad();

    public abstract void setAddMenuItemDialPad(DialPad dialPad);

    public abstract AttendantMenuAction getAddMenuItemAction();

    public abstract void setAddMenuItemAction(AttendantMenuAction action);

    public abstract void setMenuItems(AttendantMenuItemMapAdapter menuItems);

    public abstract AttendantMenuItemMapAdapter getMenuItems();

    public void removeMenuItems() {
        Iterator selected = getSelections().getAllSelected().iterator();
        Map menuItems = getAttendant().getMenuItems();
        while (selected.hasNext()) {
            String name = (String) selected.next();
            menuItems.remove(DialPad.getByName(name));
        }
    }

    public void reset() {
        getAttendant().resetToFactoryDefault();
    }

    public void commit() {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        if (!validator.getHasErrors()) {
            getDialPlanContext().storeAutoAttendant(getAttendant());
            getVxmlGenerator().generate(getAttendant());
        }
    }

    public void addMenuItem() {
        if (getAddMenuItemAction() == null) {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record("You must select an action for your new attendant menu item",
                    ValidationConstraint.REQUIRED);
        } else {
            AttendantMenuItem menuItem = new AttendantMenuItem(getAddMenuItemAction());
            getAttendant().addMenuItem(getAddMenuItemDialPad(), menuItem);
            selectNextAvailableDialpadKey();
            setAddMenuItemAction(null);
        }
    }

    /**
     * Try to select the next likely dial pad key
     */
    private void selectNextAvailableDialpadKey() {
        // set last desparate attempt
        setAddMenuItemDialPad(DialPad.POUND);

        Map menuItems = getAttendant().getMenuItems();
        for (int i = 0; i < DialPad.KEYS.length; i++) {
            DialPad key = DialPad.KEYS[i];
            // probably not pound
            if (!menuItems.containsKey(key) && key != DialPad.POUND) {
                setAddMenuItemDialPad(DialPad.KEYS[i]);
                break;
            }
        }
    }

    public void pageBeginRender(PageEvent event_) {

        AutoAttendant aa = getAttendant();
        if (aa == null) {
            aa = getDialPlanContext().newAutoAttendantWithDefaultGroup();
            aa.resetToFactoryDefault();
            setAttendant(aa);
        }
        selectNextAvailableDialpadKey();

        setMenuItems(new AttendantMenuItemMapAdapter(aa.getMenuItems()));
    }

    public String getActionName(AttendantMenuAction action) {
        return getMessages().getMessage("menuItemAction." + action.getName());
    }

    /**
     * Let's you set keys on map entries without losing your place in the iteration thru the map.
     * This is handy in tapestry when you iterate thru a list and your key values can change.
     */
    static class AttendantMenuItemMapAdapter {

        private Map m_menuItems;

        private DialPad[] m_dialPadKeys;

        private DialPad m_currentDialPadKey;

        AttendantMenuItemMapAdapter(Map menuItems) {
            m_menuItems = menuItems;
            m_dialPadKeys = (DialPad[]) menuItems.keySet().toArray(new DialPad[menuItems.size()]);
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
            AttendantMenuItem value = (AttendantMenuItem) m_menuItems.get(m_currentDialPadKey);
            m_menuItems.remove(dialPadKey);
            m_menuItems.put(dialPadKey, value);
            m_currentDialPadKey = dialPadKey;
        }

        public DialPad getCurrentMenuItemDialPadKeyAssignment() {
            return m_currentDialPadKey;
        }

        public AttendantMenuItem getCurrentMenuItem() {
            AttendantMenuItem value = (AttendantMenuItem) m_menuItems.get(m_currentDialPadKey);
            return value;
        }

        public void setCurrentMenuItem(AttendantMenuItem menuItem) {
            m_menuItems.put(m_currentDialPadKey, menuItem);
        }
    }
}
