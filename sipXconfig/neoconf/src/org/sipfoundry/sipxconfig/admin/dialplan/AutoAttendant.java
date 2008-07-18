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

import java.io.File;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class AutoAttendant extends BeanWithGroups implements NamedObject {
    public static final Log LOG = LogFactory.getLog(AutoAttendant.class);

    public static final String BEAN_NAME = "autoAttendant";

    public static final String OPERATOR_ID = "operator";

    public static final String AFTERHOUR_ID = "afterhour";

    private static final String SYSTEM_NAME_PREFIX = "xcf";

    private String m_name;

    private String m_description;

    private String m_prompt;

    private AttendantMenu m_menu = new AttendantMenu();

    private String m_systemId;

    private VxmlGenerator m_vxmlGenerator;

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

    public File getPromptFile() {
        return new File(m_vxmlGenerator.getPromptsDirectory(), m_prompt);
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public void setMenu(AttendantMenu menu) {
        m_menu = menu;
    }

    public AttendantMenu getMenu() {
        return m_menu;
    }

    public VxmlGenerator getVxmlGenerator() {
        return m_vxmlGenerator;
    }

    public void setVxmlGenerator(VxmlGenerator vxmlGenerator) {
        m_vxmlGenerator = vxmlGenerator;
    }

    public void resetToFactoryDefault() {
        setDescription(null);
        m_menu.reset(isPermanent());
    }

    @Override
    public void initialize() {
        AudioDirectorySetter audioDirectorySetter = new AudioDirectorySetter(m_vxmlGenerator.getPromptsDirectory());
        getSettings().acceptVisitor(audioDirectorySetter);
    }

    private static class AudioDirectorySetter extends AbstractSettingVisitor {
        private final String m_audioDirectory;

        public AudioDirectorySetter(String directory) {
            m_audioDirectory = directory;
        }

        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_audioDirectory);
            }
        }
    }
}
