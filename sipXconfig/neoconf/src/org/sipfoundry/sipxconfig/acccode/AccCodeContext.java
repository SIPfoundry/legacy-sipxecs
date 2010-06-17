/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;

public interface AccCodeContext extends DialingRuleProvider {

    String getAuthCodePrefix();

    void setAuthCodePrefix(String prefix);

    boolean isEnabled();
}
