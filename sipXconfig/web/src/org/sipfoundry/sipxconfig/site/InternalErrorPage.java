/**
 *
 *
 * Copyright (c) 2010 / 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.engine.ILink;

public abstract class InternalErrorPage extends org.apache.tapestry.pages.Exception {
    @InjectObject(value = "service:sipxconfig.ApplicationLifecycle")
    public abstract ApplicationLifecycle getApplicationLifecycle();

    public ILink logout(IRequestCycle cycle) {
        getApplicationLifecycle().logout();
        throw new PageRedirectException(LoginPage.PAGE);
    }
}
