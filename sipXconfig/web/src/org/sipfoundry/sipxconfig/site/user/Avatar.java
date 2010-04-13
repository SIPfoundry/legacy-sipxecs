/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.asset.ExternalAsset;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Gravatar;
import static org.apache.commons.lang.StringUtils.defaultString;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class Avatar extends BaseComponent {
    @Parameter
    public abstract User getUser();

    public IAsset getAvatarAsset() {
        Gravatar gravatar = new Gravatar(getUser());
        String url = defaultString(gravatar.getUrl(), "https://secure.gravatar.com/avatar");
        return new ExternalAsset(url, null);
    }

    public String getAvatarLinkUrl() {
        Gravatar gravatar = new Gravatar(getUser());
        return gravatar.getSignupUrl();
    }
}
