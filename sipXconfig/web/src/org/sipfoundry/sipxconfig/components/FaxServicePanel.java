/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

@ComponentClass(allowBody = false)
public abstract class FaxServicePanel extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);

    public abstract String getFaxExtension();

    public abstract void setFaxExtension(String faxExtension);

    public abstract String getFaxDid();

    public abstract void setFaxDid(String did);

    @Parameter(required = true)
    public abstract User getUser();

    public abstract void setUser(User u);

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        setFaxExtension(getUser().getFaxExtension());
        setFaxDid(getUser().getFaxDid());
    }

    public void update(User user) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        // delete the fax extension if both emails are blank
        if (StringUtils.isEmpty(getMailboxPreferences().getEmailAddress())
                && StringUtils.isEmpty(getMailboxPreferences().getAlternateEmailAddress())) {
            user.setFaxExtension(null);
            user.setFaxDid(null);
        } else {
            if (!StringUtils.isEmpty(getFaxDid()) && StringUtils.isEmpty(getFaxExtension())) {
                throw new UserException("&error.must.provide.fax.extension");
            }
            user.setFaxExtension(getFaxExtension());
            user.setFaxDid(getFaxDid());
        }
    }

    public boolean isFaxDisabled() {
        return StringUtils.isEmpty(getMailboxPreferences().getEmailAddress())
                && StringUtils.isEmpty(getMailboxPreferences().getAlternateEmailAddress())
                || !getUserSession().isAdmin();
    }
}
