/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.annotations.Bean;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class UserSchedules extends UserBasePage {

    public static final String PAGE = "user_portal/UserSchedules";

    @Bean
    public abstract SipxValidationDelegate getValidator();
}
