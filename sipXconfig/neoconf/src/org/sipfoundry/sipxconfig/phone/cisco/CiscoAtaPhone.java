/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.cisco;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;

/**
 * Support for Cisco ATA186/188 and Cisco 7905/7912
 */
public class CiscoAtaPhone extends CiscoPhone {
    public static final String BEAN_ID = "ciscoAta";

    private boolean m_isTextFormatEnabled;

    public CiscoAtaPhone() {
        super(new CiscoModel(BEAN_ID));
    }

    @Override
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        return new Evaluator(getModel().getModelId());
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new CiscoAtaDefaults(getPhoneContext().getPhoneDefaults()));
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new CiscoAtaLineDefaults(line));
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        return CiscoAtaLineDefaults.getLineInfo(getCiscoModel(), line);
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        CiscoAtaLineDefaults.setLineInfo(getCiscoModel(), line, lineInfo);
    }

    /**
     * Generate files in text format. Won't be usable by phone, but you can use cisco config tool
     * to convert manually. This is mostly for debugging
     * 
     * @param isTextFormatEnabled true to save as text, default is false
     */
    public void setTextFormatEnabled(boolean isTextFormatEnabled) {
        m_isTextFormatEnabled = isTextFormatEnabled;
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getCiscoModel().getCfgPrefix() + phoneFilename.toLowerCase();
    }

    @Override
    public void generateProfiles(ProfileLocation location) {
        ProfileFilter filter = null;
        if (!m_isTextFormatEnabled) {
            String systemDir = getPhoneContext().getSystemDirectory();
            filter = new BinaryFilter(systemDir + "/ciscoAta", getCiscoModel());
        }
        ProfileContext context = new ProfileContext(this, null);
        getProfileGenerator().generate(location, context, filter, getPhoneFilename());
    }

    public Collection<Line> getProfileLines() {
        ArrayList<Line> lines = new ArrayList(getMaxLineCount());

        lines.addAll(getLines());

        // copy in blank lines of all unused lines
        for (int i = lines.size(); i < getMaxLineCount(); i++) {
            Line line = createLine();
            line.setPhone(this);
            line.setPosition(i);
            lines.add(line);
            line.initialize();
            line.addDefaultBeanSettingHandler(new CiscoAtaLineDefaults.StubAtaLine());
        }

        return lines;
    }

    static class Evaluator implements SettingExpressionEvaluator {
        private String m_model;

        public Evaluator(String model) {
            m_model = model;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_model.matches(expression);
        }
    }
}
