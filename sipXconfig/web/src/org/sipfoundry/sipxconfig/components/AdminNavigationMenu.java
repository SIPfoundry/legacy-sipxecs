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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.web.access.WebInvocationPrivilegeEvaluator;

@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class AdminNavigationMenu extends BaseComponent {

    @Parameter
    public abstract String getPagePath();

    @Parameter
    public abstract String getLabel();

    @InjectObject(value = "spring:webInvocationPrivilegeEvaluator")
    public abstract WebInvocationPrivilegeEvaluator getPrivilegeEvaluator();

    public boolean isAllowed() {
        return getPrivilegeEvaluator().isAllowed("sipxconfig", "/" + getPagePath(), null,
                SecurityContextHolder.getContext().getAuthentication());
    }
}
