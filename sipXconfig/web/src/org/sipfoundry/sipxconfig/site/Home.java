/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.html.BasePage;

public abstract class Home extends BasePage {
    public static final String PAGE = "Home";

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();
}
