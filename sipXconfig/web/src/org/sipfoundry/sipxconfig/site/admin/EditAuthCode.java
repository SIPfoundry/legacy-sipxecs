/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCodeManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAuthCode extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/EditAuthCode";

    private static final Log LOG = LogFactory.getLog(EditAuthCode.class);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:authCodeManager")
    public abstract AuthCodeManager getAuthCodeManager();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    public abstract AuthCode getAuthCode();

    public abstract void setAuthCode(AuthCode code);

    @Persist
    public abstract Integer getAuthCodeId();

    public abstract void setAuthCodeId(Integer codeId);

    public void pageBeginRender(PageEvent event) {
        AuthCode code = getAuthCode();
        if (code != null) {
            if (!code.isNew()) {
                setAuthCodeId(code.getId());
            }
            return;
        }
        if (getAuthCodeId() != null) {
            code = getAuthCodeManager().getAuthCode(getAuthCodeId());
        } else {
            code = getAuthCodeManager().newAuthCode();
        }
        setAuthCode(code);
    }

    public void saveAuthCode() {
        LOG.info("ENTERED EditAuthCode::saveAuthCode() : ");
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        AuthCode code = getAuthCode();
        getAuthCodeManager().saveAuthCode(code);
        setAuthCodeId(code.getId());
    }

}
