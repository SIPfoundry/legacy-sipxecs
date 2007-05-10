/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.setting.SimpleDefinitionsEvaluator;

/**
 * Base class for managed phone subclasses
 */
public abstract class Phone extends BeanWithGroups implements Device {
    // public because of checkstyle
    public static final String PHONE_CONSTANT = "phone";

    public static final String GROUP_RESOURCE_ID = PHONE_CONSTANT;

    private String m_description;

    private String m_serialNumber;

    private List<Line> m_lines = Collections.EMPTY_LIST;

    private PhoneContext m_phoneContext;

    private ModelSource<PhoneModel> m_modelSource;

    private ProfileGenerator m_profileGenerator;

    private SipService m_sip;

    private String m_phoneTemplate;

    private String m_beanId;

    private String m_modelId;

    private PhoneModel m_model;

    private DeviceVersion m_version;

    protected Phone(String beanId) {
        m_beanId = beanId;
    }

    protected Phone(PhoneModel model) {
        setModel(model);
    }

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
        m_model = null;
    }

    public void setModel(PhoneModel model) {
        m_model = model;
        m_modelId = model.getModelId();
        m_beanId = model.getBeanId();
    }

    public PhoneModel getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (m_modelId == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(m_modelId);
        return m_model;
    }

    public DeviceVersion getDeviceVersion() {
        return m_version;
    }

    public void setDeviceVersion(DeviceVersion version) {
        m_version = version;
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSipService() {
        return m_sip;
    }

    public void setProfileGenerator(ProfileGenerator profileGenerator) {
        m_profileGenerator = profileGenerator;
    }

    public ProfileGenerator getProfileGenerator() {
        return m_profileGenerator;
    }

    protected Setting loadSettings() {
        return getModelFilesContext().loadDynamicModelFile("phone.xml", getBeanId(),
                getSettingsEvaluator());
    }

    protected Setting loadLineSettings() {
        return getModelFilesContext().loadDynamicModelFile("line.xml", getBeanId(),
                getSettingsEvaluator());
    }

    /**
     * If SimepleDefinitionEvaluator is not what you need override this function to provide your
     * own expression evaluator to be used when loading settings model.
     * 
     * @return reusable copy of expression evaluator
     */
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        Set defines = getModel().getDefinitions();
        if (getDeviceVersion() != null) {
            defines.add(getDeviceVersion().getVersionId());
        }
        return new SimpleDefinitionsEvaluator(defines);
    }

    /**
     * Each subclass must decide how as much of this generic line information translates into its
     * own setting model.
     */
    protected abstract void setLineInfo(Line line, LineInfo lineInfo);

    /**
     * Each subclass must decide how as much of this generic line information can be contructed
     * from its own setting model.
     */
    protected abstract LineInfo getLineInfo(Line line);

    /**
     * Overwrite to generate profiles
     * 
     * @return null - which means do not generate profiles
     */
    public String getPhoneFilename() {
        return null;
    }

    /**
     * Default implementation - generates a single profile file based in provided file name and
     * phone template
     */
    public void generateProfiles(ProfileLocation location) {
        String profileFileName = getPhoneFilename();
        m_profileGenerator.generate(location, createContext(), profileFileName);
    }

    protected ProfileContext createContext() {
        return new ProfileContext(this, getPhoneTemplate());
    }

    public Line findByUsername(String username) {
        for (Line l : getLines()) {
            User user = l.getUser();
            if (user != null && user.getUserName().equals(username)) {
                return l;
            }
        }
        return null;
    }

    public Line findByUri(String uri) {
        for (Line l : getLines()) {
            String candidate = l.getUri();
            if (candidate.equals(uri)) {
                return l;
            }
        }
        return null;
    }

    public void removeLine(Line line) {
        getLines().remove(line);
        DataCollectionUtil.updatePositions(getLines());
    }

    /**
     * Removes profiles generated by generateProfiles
     */
    public void removeProfiles(ProfileLocation location) {
        location.removeProfile(getPhoneFilename());
    }

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    /**
     * @throws RestartException if it cannot complete the operation
     */
    public void restart() {
    }

    protected void sendCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        Line line = getLine(0);

        // XCF-1059
        String outboundProxy = m_phoneContext.getPhoneDefaults().getDomainName();

        String outboundProxyPort = m_phoneContext.getPhoneDefaults().getProxyServerSipPort();
        m_sip.sendCheckSync(line.getUri(), outboundProxy, outboundProxyPort);
    }

    /**
     * @return ids used in PhoneFactory
     */
    public String getBeanId() {
        return m_beanId;
    }

    /**
     * Internal, do not call this method. Hibnerate property declared update=false, but still
     * required method be defined.
     */
    @SuppressWarnings("unused")
    public void setBeanId(String illegal_) {
    }

    public String getModelId() {
        return m_modelId;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

    public List<Line> getLines() {
        return m_lines;
    }

    public void setLines(List<Line> lines) {
        m_lines = lines;
    }

    public void addLine(Line line) {
        if (m_lines == Collections.EMPTY_LIST) {
            m_lines = new ArrayList<Line>();
        }
        int max = getModel().getMaxLineCount();
        if (m_lines.size() >= max) {
            throw new MaxLinesException("Maximum number of allowed lines is " + max);
        }
        line.setPhone(this);
        line.setPosition(m_lines.size());
        m_lines.add(line);
        line.initialize();
    }

    public static class MaxLinesException extends UserException {
        MaxLinesException(String msg) {
            super(msg);
        }
    }

    public Line getLine(int position) {
        return m_lines.get(position);
    }

    @SuppressWarnings("unused")
    public void initializeLine(Line line) {
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public Line createLine() {
        Line line = new Line();
        line.setPhone(this);
        line.setModelFilesContext(getModelFilesContext());
        return line;
    }

    public void setPhoneModelSource(ModelSource<PhoneModel> modelSource) {
        m_modelSource = modelSource;
    }

    /**
     * Find a phone user. By convention phone user is a user associated with the phone first line.
     * 
     */
    public User getPrimaryUser() {
        List<Line> lines = getLines();
        if (lines.isEmpty()) {
            return null;
        }
        return lines.get(0).getUser();
    }
}
