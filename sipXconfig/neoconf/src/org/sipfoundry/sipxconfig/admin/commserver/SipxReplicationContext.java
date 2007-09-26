/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigurationFile;
import org.springframework.context.ApplicationEvent;

public interface SipxReplicationContext {
    void generate(DataSet dataSet);

    void generateAll();

    void replicate(ConfigurationFile xmlFile);

    String getXml(DataSet dataSet);

    /**
     * This function will publish application event - in case the application is done lazily it
     * will publish the even only after everything has been replicated
     * 
     * @param event event to be published
     */
    void publishEvent(ApplicationEvent event);
}
