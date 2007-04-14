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

import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ConferencesPanel extends TablePanel {
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    protected void removeRows(Collection selectedRows) {
        getConferenceBridgeContext().removeConferences(selectedRows);
    }
}
