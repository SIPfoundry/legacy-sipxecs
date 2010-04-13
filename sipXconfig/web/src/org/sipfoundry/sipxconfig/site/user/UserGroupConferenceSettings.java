/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.BeanPropertySelectionModel;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserGroupConferenceSettings extends BaseComponent {

    private static final String SETTING_PATH_CONFERENCE_ENABLED = "conference/enabled";
    private static final String SETTING_PATH_CONFERENCE_BRIDGE_ID = "conference/bridgeId";
    private static final String SETTING_PATH_CONFERENCE_PREFIX = "conference/prefix";

    private static final Log LOG = LogFactory.getLog(UserGroupConferenceSettings.class);

    @Parameter(required = true)
    public abstract Setting getSettings();

    @Parameter(required = true)
    public abstract Group getGroup();

    @Parameter(required = true)
    public abstract IValidationDelegate getValidator();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public IPropertySelectionModel getConferenceBridgesModel() {
        List<Bridge> bridges = getConferenceBridgeContext().getBridges();
        BeanPropertySelectionModel bridgeModel = new BeanPropertySelectionModel(bridges, "name");
        ExtraOptionModelDecorator model = new ExtraOptionModelDecorator();
        model.setModel(bridgeModel);
        model.setExtraLabel(getMessages().getMessage("label.bridgeSelect.selectBridge"));
        model.setExtraOption(null);
        return model;
    }

    public Bridge getBridge() {
        Bridge bridge = null;
        Integer bridgeId = (Integer) getSettings().getSetting(SETTING_PATH_CONFERENCE_BRIDGE_ID).getTypedValue();
        if (bridgeId != null) {
            try {
                bridge = getConferenceBridgeContext().loadBridge(bridgeId);
            } catch (HibernateObjectRetrievalFailureException horfe) {
                LOG.warn(String.format("User group \"%s\" references a non-existent bridge id: %d; setting to null.",
                        getGroup().getName(), bridgeId));
            }
        }

        return bridge;
    }

    public void setBridge(Bridge bridge) {
        getSettings().getSetting(SETTING_PATH_CONFERENCE_BRIDGE_ID).setTypedValue(
                bridge != null ? bridge.getId() : bridge);
    }

    public Boolean isConferenceEnabled() {
        return Boolean.valueOf(getSettings().getSetting(SETTING_PATH_CONFERENCE_ENABLED).getValue());
    }

    public void setConferenceEnabled(Boolean enabled) {
        getSettings().getSetting(SETTING_PATH_CONFERENCE_ENABLED).setTypedValue(enabled.toString());
    }

    public String getConferencePrefix() {
        return getSettings().getSetting(SETTING_PATH_CONFERENCE_PREFIX).getValue();
    }

    public void setConferencePrefix(String prefix) {
        getSettings().getSetting(SETTING_PATH_CONFERENCE_PREFIX).setValue(prefix);
    }

    public boolean validateConferenceSettings() {
        IValidationDelegate validator = TapestryUtils.getValidator(getPage());
        boolean valid = true;
        if (isConferenceEnabled()) {
            // Require that an offset and bridge are selected
            if (getConferencePrefix() == null) {
                validator.record(new ValidatorException(getMessages().getMessage("message.prefixRequired")));
                valid = false;
            } else if (getBridge() == null) {
                validator.record(new ValidatorException(getMessages().getMessage("message.bridgeRequired")));
                valid = false;
            }
        }

        return valid;
    }


    public void ok(IRequestCycle cycle) {
        if (validateConferenceSettings()) {
            GroupSettings groupPage = (GroupSettings) getPage();
            groupPage.ok(cycle);
        }
    }

    public void apply() {
        if (validateConferenceSettings()) {
            GroupSettings groupPage = (GroupSettings) getPage();
            groupPage.apply();
        }
    }

    public void cancel(IRequestCycle cycle) {
        GroupSettings groupPage = (GroupSettings) getPage();
        groupPage.cancel(cycle);
    }
}
