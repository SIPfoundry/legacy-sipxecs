/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public interface ConditionalSetting extends Setting {
    public String getIf();
    public void setIf(String if1);
    public String getUnless();
    public void setUnless(String unless);
}
