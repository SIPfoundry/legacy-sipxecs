/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.engine.RequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ActiveConference;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.BridgeConferenceIdentity;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiConnectException;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConferencesPanel extends TablePanel {
    /**
     * This component can display conferences per a given bridge or a given set of conferences
     * Please provide one of these two parameters: 'bridge' or 'conferences'
     *
     * If a set of conferences is provided then the filter option must be set to false
     * When it is wanted to display the conferences per a given bridge we recomend to set the filter
     * option to true. In this way you can filter the conferences as desired (per user group or search query).
    */
    private static final Log LOG = LogFactory.getLog(ConferencesPanel.class);

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @Override
    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Bean
    public abstract SelectMap getSelections();

    @Parameter(required = true)
    public abstract IActionListener getAddListener();

    @Parameter(required = true)
    public abstract IActionListener getSelectListener();

    @Parameter(required = true)
    public abstract IActionListener getSelectActiveListener();

    @Parameter(required = false)
    public abstract Bridge getBridge();

    @Parameter(required = false)
    public abstract List<Conference> getConferences();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getEnableAdd();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getShowOwner();

    @Parameter(defaultValue = "false")
    public abstract boolean getRenderFilter();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getEnableDelete();

    @Parameter
    public abstract boolean isChanged();

    @Asset("/images/lock.png")
    public abstract IAsset getLockedIcon();

    public abstract Map<Conference, ActiveConference> getActiveConferenceMap();
    public abstract void setActiveConferenceMap(Map<Conference, ActiveConference> activeConferenceMap);

    @Override
    public abstract Collection getRowsToDelete();

    public abstract Conference getCurrentRow();

    public abstract String getQueryText();

    public abstract Integer getSelectedGroupId();

    public abstract boolean getSearchMode();

    @InjectObject(value = "spring:identity")
    public abstract BridgeConferenceIdentity getIdentity();

    private void loadActiveConferenceMap() {
        try {
            if (getBridge() != null) {
                setActiveConferenceMap(getActiveConferenceContext().getActiveConferencesMap(getBridge()));
            }
        } catch (FreeswitchApiConnectException face) {
            LOG.error("Couldn't connect to FreeSWITCH to get active conferences!", face);
        }
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);

        if (getActiveConferenceMap() == null) {
            loadActiveConferenceMap();
        }
    }

    public Object getFilteredConferences() {
        if (!getRenderFilter() && getConferences() != null) {
            return getConferences();
        }

        ConferencesTableModel conferenceTableModel;
        conferenceTableModel = new ConferencesTableModel(getConferenceBridgeContext(), getBridge());
        conferenceTableModel.setGroupId(getSelectedGroupId());
        conferenceTableModel.setQueryText(getQueryText());
        conferenceTableModel.setSearchMode(getSearchMode());
        getIdentity().setBridge(getBridge());
        conferenceTableModel.setIdentity(getIdentity());

        return conferenceTableModel;
    }

    void recordFailure(UserException ue) {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        if (validator instanceof SipxValidationDelegate) {
            SipxValidationDelegate v = (SipxValidationDelegate) validator;
            v.record(ue, getMessages());
        }
    }

    @Override
    protected void removeRows(Collection selectedRows) {
        getConferenceBridgeContext().removeConferences(selectedRows);
    }

    public String getTableColumns() {
        // "* name,!owner,enabled,extension,description,active"
        StringBuilder columns = new StringBuilder("* name,");
        if (getShowOwner()) {
            columns.append("!owner,");
        }
        columns.append("enabled,extension,description,!active");

        return columns.toString();
    }

    public void calculateActiveValue(RequestCycle cycle, int id) {
        Conference conference = getConferenceBridgeContext().loadConference(id);
        if (!conference.isEnabled()) {
            return;
        }

        try {
            int activeCount = getActiveConferenceContext().getConferenceMembers(conference)
                    .size();
            LOG.info("Conference: " + conference.getName() + " Conference: " + activeCount);
            ActiveValue.setActiveCount(cycle, activeCount);
        } catch (FreeswitchApiException fae) {
            // TODO better UI to mark the bridge in the UI as unreachable
            LOG.error("Couldn't connect to FreeSWITCH to fetch active conferences", fae);
        }
    }

    public boolean isConferenceLocked(Conference conference) {
        try {
            if (getActiveConferenceMap() == null) {
                if (getBridge() != null) {
                    setActiveConferenceMap(getActiveConferenceContext().getActiveConferencesMap(getBridge()));
                } else {
                    setActiveConferenceMap(
                            getActiveConferenceContext().getActiveConferencesMap(conference.getBridge()));
                }

            }

            ActiveConference activeConference = getActiveConferenceMap().get(conference);
            return (activeConference == null) ? false : activeConference.isLocked();
        } catch (FreeswitchApiConnectException face) {
            LOG.error("Couldn't connect to FreeSWITCH to get conference lock status", face);
            return false;
        }
    }

    public void deleteConferences() {
        Collection rowsToDelete = getRowsToDelete();
        if (rowsToDelete != null) {
            removeRows(rowsToDelete);
        }
    }

    public abstract class Action implements Closure {
        private final String m_msgSuccess;

        public Action(String msgSuccess) {
            m_msgSuccess = getMessages().getMessage(msgSuccess);
        }

        public void execute(Object id) {
            Conference conference = getConferenceBridgeContext()
                    .loadConference((Serializable) id);
            execute(conference);
            TapestryUtils.recordSuccess(ConferencesPanel.this, m_msgSuccess);
        }

        public abstract void execute(Conference conference);
    }

    private void forAllConferences(Closure closure) {
        Collection<Integer> selected = getSelections().getAllSelected();
        if (selected.isEmpty()) {
            return;
        }
        CollectionUtils.forAllDo(selected, closure);
    }

    public void lockConferences() {
        Closure lock = new Action("msg.success.lock") {
            @Override
            public void execute(Conference conference) {
                if (getActiveConferenceContext().getActiveConference(conference) != null) {
                    getActiveConferenceContext().lockConference(conference);
                    loadActiveConferenceMap();
                } else {
                    recordFailure(new UserException("&error.lockEmpty", conference.getName()));
                }
            }
        };
        forAllConferences(lock);
    }

    public void unlockConferences() {
        Closure lock = new Action("msg.success.unlock") {
            @Override
            public void execute(Conference conference) {
                if (getActiveConferenceContext().getActiveConference(conference) != null) {
                    getActiveConferenceContext().unlockConference(conference);
                    loadActiveConferenceMap();
                } else {
                    recordFailure(new UserException("&error.unlockEmpty", conference.getName()));
                }
            }

        };
        forAllConferences(lock);
    }
}
