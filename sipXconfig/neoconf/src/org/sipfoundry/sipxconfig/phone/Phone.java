/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone;

import static org.sipfoundry.commons.mongo.MongoConstants.SERIAL_NUMBER;
import static org.sipfoundry.commons.mongo.MongoConstants.TIMESTAMP;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.sip.SipService;

/**
 * Base class for managed phone subclasses
 */
public abstract class Phone extends Device implements Replicable {
    public static final Log LOG = LogFactory.getLog(Phone.class);

    public static final String URI_IN_PREFIX = "~~in~";

    // public because of checkstyle
    public static final String PHONE_CONSTANT = "phone";

    public static final String GROUP_RESOURCE_ID = PHONE_CONSTANT;

    private static final String PHONE_SIP_EXCEPTION = "&phone.sip.exception";
    private static final String PHONE_LINE_NOT_VALID = "&phone.line.not.valid";
    private static final String E911_SETTING_PATH = "e911/location";

    private String m_description;

    private List<Line> m_lines = Collections.emptyList();

    private PhoneContext m_phoneContext;

    private PhonebookManager m_phonebookManager;

    private ModelSource<PhoneModel> m_modelSource;

    private SipService m_sip;

    private PhoneModel m_model;

    private String m_profileDir;

    private FeatureManager m_featureManager;

    protected Phone() {
    }

    public void setModel(PhoneModel model) {
        m_model = model;
        setModelId(model.getModelId());
        setBeanId(model.getBeanId());
    }

    @Override
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

    @Override
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

    public String getAdditionalPhoneSettings() {
        return null;
    }

    public void setAdditionalPhoneSettings(String additionalSettings) {
    }

    public List<String> getLinePaths() {
        return null;
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
    }

    protected void sendCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            throw new RestartException(PHONE_LINE_NOT_VALID);
        }

        Line line = getLine(0);
        try {
            m_sip.sendCheckSync(line.getAddrSpec());
        } catch (RuntimeException ex) {
            throw new RestartException(PHONE_SIP_EXCEPTION);
        }
    }

    // If device challenges check-sync NOTIFY, use this method
    protected void sendAuthorizedCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            Line line = createSpecialPhoneProvisionUserLine();
            try {
                m_sip.sendCheckSync(line.getAddrSpec(), line.getUserName(), getSerialNumber(), line.getUser()
                        .getSipPassword());
            } catch (RuntimeException ex) {
                throw new RestartException(PHONE_SIP_EXCEPTION);
            }
        } else {
            Line line = getLine(0);
            try {
                m_sip.sendCheckSync(line.getAddrSpec(), line.getUserName(), getSerialNumber(), line.getUser()
                        .getSipPassword());
            } catch (RuntimeException ex) {
                /*
                 * If the previous attempt didn't work, perhaps the phone has been
                 * auto-provisioned, but has not yet been restarted. If sending a restart to the
                 * defined line doesn't work, fall back to the auto-provision user.
                 */
                Line linedefault = createSpecialPhoneProvisionUserLine();
                try {
                    m_sip.sendCheckSync(linedefault.getAddrSpec(), linedefault.getUserName(), getSerialNumber(),
                            linedefault.getUser().getSipPassword());
                } catch (RuntimeException exc) {
                    throw new RestartException(PHONE_SIP_EXCEPTION);
                }
            }
        }
    }

    // To send check-sync to the device that is authenticated with instrument id
    protected void sendCheckSyncToMac() {
        try {
            m_sip.sendCheckSync(getInstrumentAddrSpec());
        } catch (RuntimeException ex) {
            throw new RestartException(PHONE_SIP_EXCEPTION);
        }
    }

    public String getInstrumentAddrSpec() {
        String domain = getPhoneContext().getPhoneDefaults().getDomainName();
        return SipUri.format(URI_IN_PREFIX + getSerialNumber(), domain, false);
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
            throw new MaxLinesException(String.valueOf(max));
        }
        line.setPhone(this);
        m_lines.add(line);
        line.initialize();
    }

    public static class MaxLinesException extends UserException {
        MaxLinesException(String msg) {
            super("&error.maxLinesException", msg);
        }
    }

    public Line getLine(int position) {
        return m_lines.get(position);
    }

    public void initializeLine(@SuppressWarnings("unused") Line line) {
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
     */
    public User getPrimaryUser() {
        List<Line> lines = getLines();
        if (lines.isEmpty()) {
            return null;
        }
        if (lines.get(0) == null) {
            /*
             * Note that it is possible although very improbable to have a non empty list of lines
             * but line 0 to be null (a line on position 1). This is a misconfiguration, but we
             * need to take care of this possibility.
             */
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

    public Line createSpecialPhoneProvisionUserLine() {
        Line line = createLine();
        line.setUser(m_phoneContext.createSpecialPhoneProvisionUser(getSerialNumber()));
        line.setPhone(this);
        return line;
    }

    public Collection< ? extends PhoneModel> getModelIdsForSelection(String beanId) {
        return null;
    }

    public Integer getE911LocationId() {
        if (getSettingTypedValue(E911_SETTING_PATH) == null) {
            return null;
        }
        Integer id = (Integer) getSettingTypedValue(E911_SETTING_PATH);
        if (id < 0) {
            LOG.error("Database is in bad state, E911 location defined is wrong! Please review!");
        }
        return id;
    }

    public void setE911LocationId(Integer id) {
        if (id != null && id < 0) {
            return;
        }
        setSettingTypedValue(E911_SETTING_PATH, id);
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.E911);
        return ds;
    }

    @Override
    public String getIdentity(String domainName) {
        return null;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(SERIAL_NUMBER, getSerialNumber());
        props.put(TIMESTAMP, System.currentTimeMillis());
        return props;
    }

    @Override
    public String getEntityName() {
        return PHONE_CONSTANT;
    }

    @Override
    public boolean isReplicationEnabled() {
        return true;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public FeatureManager getFeatureManager() {
        return m_featureManager;
    }
}
