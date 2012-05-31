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

import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.defaultString;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.asset.ExternalAsset;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.commons.userdb.profile.AvatarUploadException;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class Avatar extends BaseComponent {
    @Parameter
    public abstract User getUser();

    public abstract IUploadFile getUploadFile();

    @InjectObject(value = "spring:userProfileService")
    public abstract UserProfileService getUserAvatarService();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    public IAsset getAvatarAsset() {
        String url = EMPTY;
        if (getUser().getUserProfile() != null) {
            url = defaultString(getUser().getUserProfile().getAvatar(), EMPTY);
        }
        return new ExternalAsset(url, null);
    }

    public void uploadAvatar() {
        if (getUploadFile() != null) {
            try {
                getUserAvatarService().saveAvatar(getUser().getUserName(), getUploadFile().getStream());
            } catch (AvatarUploadException e) {
                TapestryUtils.getValidator(getPage()).record(
                        new ValidatorException(getMessages().getMessage("err.avatar.upload")));
            }
        }
    }

    public void extAvatar() {
        getCoreContext().saveUser(getUser());
    }
}
