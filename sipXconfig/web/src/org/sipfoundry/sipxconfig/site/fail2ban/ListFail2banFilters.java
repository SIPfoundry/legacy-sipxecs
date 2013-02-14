/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.fail2ban;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.fail2ban.Fail2banManager;
import org.sipfoundry.sipxconfig.fail2ban.Fail2banSettings;

public abstract class ListFail2banFilters extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "fail2ban/ListFail2banFilters";

    @InjectObject("spring:fail2banManager")
    public abstract Fail2banManager getFail2banManager();

    public abstract Fail2banSettings getSettings();

    public abstract void setSettings(Fail2banSettings settings);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue(value = "literal:sipRules")
    public abstract String getTab();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getFail2banManager().getSettings());
        }
    }

    public void apply(IRequestCycle cycle) {
        getFail2banManager().saveSettings(getSettings());
    }
}
