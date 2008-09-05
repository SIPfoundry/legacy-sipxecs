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

import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.sip.SipService;

/**
 * Base class for managed phone subclasses
 */
public abstract class Phone extends Device {
    // public because of checkstyle
    public static final String PHONE_CONSTANT = "phone";

    public static final String GROUP_RESOURCE_ID = PHONE_CONSTANT;

    private String m_description;

    private List<Line> m_lines = Collections.EMPTY_LIST;

    private PhoneContext m_phoneContext;

    private PhonebookManager m_phonebookManager;

    private ModelSource<PhoneModel> m_modelSource;

    private SipService m_sip;

    private PhoneModel m_model;

    private String m_profileDir;

    protected Phone() {
    }

    public void setModel(PhoneModel model) {
        m_model = model;
        setModelId(model.getModelId());
        setBeanId(model.getBeanId());
    }

    public PhoneModel getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (getModelId() == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(getModelId());
        return m_model;
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSipService() {
        return m_sip;
    }

    protected Setting loadSettings() {
        PhoneModel model = getModel();
        return getModelFilesContext().loadDynamicModelFile(model.getSettingsFile(), model.getModelDir(),
                getSettingsEvaluator());
    }

    protected Setting loadLineSettings() {
        PhoneModel model = getModel();
        return getModelFilesContext().loadDynamicModelFile(model.getLineSettingsFile(), model.getModelDir(),
                getSettingsEvaluator());
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

    protected void sendCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        Line line = getLine(0);

        m_sip.sendCheckSync(line.getAddrSpec());
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
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

    public PhonebookManager getPhonebookManager() {
        return m_phonebookManager;
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    public String getProfileDir() {
        return m_profileDir;
    }

    public void setProfileDir(String profileDir) {
        m_profileDir = profileDir;
    }

}
