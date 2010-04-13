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
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.springframework.beans.factory.annotation.Required;

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

    private String m_promptsDirectory;

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

    public void setMenu(AttendantMenu menu) {
        m_menu = menu;
    }

    public AttendantMenu getMenu() {
        return m_menu;
    }

    public void resetToFactoryDefault() {
        setDescription(null);
        m_menu.reset(isPermanent());
    }

    @Required
    public void setPromptsDirectory(String promptsDirectory) {
        m_promptsDirectory = promptsDirectory;
    }

    public String getPromptsDirectory() {
        return m_promptsDirectory;
    }

    public File getPromptFile() {
        return new File(m_promptsDirectory, m_prompt);
    }

    /**
     * updates current prompt file with a new one from the given location
     *
     * @param sourceDir new prompt location
     * @throws IOException - exception if file read fails
     */
    public void updatePrompt(File sourceDir) throws IOException {
        File newPrompt = new File(sourceDir, m_prompt);
        FileUtils.copyFileToDirectory(newPrompt, new File(m_promptsDirectory));
    }

    @Override
    public void initialize() {
        AudioDirectorySetter audioDirectorySetter = new AudioDirectorySetter(m_promptsDirectory);
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

    public static Integer getIdFromSystemId(String attendantId) {
        if (StringUtils.startsWith(attendantId, SYSTEM_NAME_PREFIX)) {
            return Integer.parseInt(StringUtils.substring(attendantId, SYSTEM_NAME_PREFIX.length() + 1));
        }
        return null;
    }
}
