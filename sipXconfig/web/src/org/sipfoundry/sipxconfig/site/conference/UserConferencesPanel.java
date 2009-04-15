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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.site.UserSession;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserConferencesPanel extends BaseComponent {
    
    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();
    
    @InjectPage(EditConference.PAGE)
    public abstract EditConference getEditConferencePage();
    
    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();    
    
    public IPage addConference() {
        EditConference editConference = getEditConferencePage();
        editConference.setBridgeId(null);
        editConference.setConferenceId(null);
        editConference.setOwnerId(getUser().getId());
        editConference.setReturnPage(getPage());
        return editConference;
    }       
    
    @Parameter(required = true)
    public abstract void setUser(User user);
    public abstract User getUser();
    
    public abstract Conference getCurrentRow();
    public abstract void setCurrentRow(Conference currentRow);
 
    public IPage selectConference(Integer conferenceId) {
        return activateEditConferencePage(conferenceId, EditConference.TAB_CONFIG);
    }
    
    public IPage selectActiveConference(Integer conferenceId) {
        return activateEditConferencePage(conferenceId, EditConference.TAB_PARTICIPANTS);
    }
    
    private IPage activateEditConferencePage(Integer conferenceId, String tab) {
        EditConference editConferencePage = getEditConferencePage();
        editConferencePage.setTab(tab);
        editConferencePage.setConferenceId(conferenceId);
        editConferencePage.setReturnPage(getPage());
        return editConferencePage;
    }
    
}
