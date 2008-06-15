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

import java.util.Collection;

import org.apache.tapestry.IActionListener;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ConferencesPanel extends TablePanel {

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Bean
    public abstract SelectMap getSelections();

    @Parameter(required = true)
    public abstract IActionListener getAddListener();

    @Parameter(required = true)
    public abstract IActionListener getSelectListener();

    @Parameter(required = true)
    public abstract Collection<Conference> getConferences();

    @Parameter(required = false)
    public abstract boolean isChanged();

    public abstract Collection getRowsToDelete();

    public abstract Conference getCurrentRow();

    protected void removeRows(Collection selectedRows) {
        getConferenceBridgeContext().removeConferences(selectedRows);
    }
}
