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

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.engine.RequestCycle;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConferencesPanel extends TablePanel {
    private static final Log LOG = LogFactory.getLog(ConferencesPanel.class);

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @Bean
    public abstract SelectMap getSelections();

    @Parameter(required = true)
    public abstract IActionListener getAddListener();

    @Parameter(required = true)
    public abstract IActionListener getSelectListener();

    @Parameter(required = true)
    public abstract IActionListener getSelectActiveListener();

    @Parameter(required = true)
    public abstract Collection<Conference> getConferences();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getShowOwner();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getEnableDelete();
    
    @Parameter
    public abstract boolean isChanged();

    public abstract Collection getRowsToDelete();

    public abstract Conference getCurrentRow();

    protected void removeRows(Collection selectedRows) {
        getConferenceBridgeContext().removeConferences(selectedRows);
    }
    
    public String getTableColumns() {
        // "* name,!owner,enabled,extension,description,active"
        StringBuilder columns = new StringBuilder("* name,");
        if (getShowOwner()) {
            columns.append("!owner,");
        }
        columns.append("enabled,extension,description,active");
        
        return columns.toString();
    }

    public void calculateActiveValue(RequestCycle cycle, int id) {
        Conference conference = getConferenceBridgeContext().loadConference(id);
        if (!conference.isEnabled()) {
            return;
        }

        try {
            int activeCount = getActiveConferenceContext().getConferenceMembers(conference).size();
            LOG.info("Conference: " + conference.getName() + " Conference: " + activeCount);
            ActiveValue.setActiveCount(cycle, activeCount);
        } catch (FreeswitchApiException fae) {
            // TODO better UI to mark the bridge in the UI as unreachable
            LOG.error("Couldn't connect to FreeSWITCH to fetch active conferences", fae);
        }
    }

    public abstract class Action implements Closure {
        private String m_msgSuccess;

        public Action(String msgSuccess) {
            m_msgSuccess = getMessages().getMessage(msgSuccess);
        }

        public void execute(Object id) {
            Conference conference = getConferenceBridgeContext().loadConference((Serializable) id);
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
            public void execute(Conference conference) {
                getActiveConferenceContext().lockConference(conference);
            }

        };
        forAllConferences(lock);
    }

    public void unlockConferences() {
        Closure lock = new Action("msg.success.unlock") {
            public void execute(Conference conference) {
                getActiveConferenceContext().unlockConference(conference);
            }

        };
        forAllConferences(lock);
    }
}
