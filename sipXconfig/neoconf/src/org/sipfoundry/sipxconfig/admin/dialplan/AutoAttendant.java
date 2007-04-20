/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AutoAttendant extends BeanWithGroups implements NamedObject {
    public static final Log LOG = LogFactory.getLog(AutoAttendant.class);

    public static final String BEAN_NAME = "autoAttendant";

    public static final String OPERATOR_ID = "operator";

    public static final String AFTERHOUR_ID = "afterhour";

    private static final String SYSTEM_NAME_PREFIX = "xcf";

    private String m_name;

    private String m_description;

    private String m_prompt;

    private Map m_menuItems;

    private String m_systemId;

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxvxml/autoattendant.xml");
    }

    /**
     * This is the name passed to the mediaserver cgi to locate the correct auto attendant.
     * Technically it's invalid until saved to database.
     */
    public String getSystemName() {
        if (getSystemId() != null) {
            return getSystemId();
        }
        return SYSTEM_NAME_PREFIX + getId().toString();
    }

    /**
     * Certain auto attendants like the operator are system known.
     * 
     * @return null if attendant is not system known
     */
    public String getSystemId() {
        return m_systemId;
    }

    public void setSystemId(String systemId) {
        m_systemId = systemId;
    }

    public boolean isOperator() {
        return OPERATOR_ID.equals(getSystemId());
    }

    public boolean isAfterhour() {
        return AFTERHOUR_ID.equals(getSystemId());
    }

    /**
     * Check is this is a permanent attendant.
     * 
     * You cannot delete operator or afterhour attendant.
     * 
     * @return true for operator or afterhour, false otherwise
     */
    public boolean isPermanent() {
        return isOperator() || isAfterhour();
    }

    public String getDescription() {
        return m_description;
    }

    public String getScriptFileName() {
        return "autoattendant-" + getSystemName() + ".vxml";
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getPrompt() {
        return m_prompt;
    }

    public void setPrompt(String prompt) {
        m_prompt = prompt;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    /**
     * @return map of AttendantMenuItems where the dialpad keys DialPad objects representing keys
     *         0-9,* and #
     */
    public Map getMenuItems() {
        return m_menuItems;
    }

    public void setMenuItems(Map menuItems) {
        m_menuItems = menuItems;
    }

    public void addMenuItem(DialPad key, AttendantMenuItem menuItem) {
        if (m_menuItems == null) {
            m_menuItems = new TreeMap();
        }

        m_menuItems.put(key, menuItem);
    }

    public void resetToFactoryDefault() {
        if (m_menuItems != null) {
            m_menuItems.clear();
        }
        setDescription(null);
        if (isPermanent()) {
            addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
            addMenuItem(DialPad.NUM_9, new AttendantMenuItem(AttendantMenuAction.DIAL_BY_NAME));
            addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.REPEAT_PROMPT));
            addMenuItem(DialPad.POUND, new AttendantMenuItem(AttendantMenuAction.VOICEMAIL_LOGIN));
        } else {
            addMenuItem(DialPad.NUM_0, new AttendantMenuItem(AttendantMenuAction.OPERATOR));
            addMenuItem(DialPad.STAR, new AttendantMenuItem(AttendantMenuAction.REPEAT_PROMPT));
        }
    }
}
