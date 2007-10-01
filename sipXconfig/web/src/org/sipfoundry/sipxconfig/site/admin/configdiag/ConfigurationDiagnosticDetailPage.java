/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.configdiag;

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnostic;

public abstract class ConfigurationDiagnosticDetailPage extends BasePage {
    public static final String PAGE = "ConfigurationDiagnosticDetailPage";
    
    public abstract ConfigurationDiagnostic getConfigurationDiagnostic();
    public abstract void setConfigurationDiagnostic(ConfigurationDiagnostic diagnostic);
}
