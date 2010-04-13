/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.time;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.time.TimeManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditNtpSettings extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract TimeManager getTimeManager();

    public abstract List<String> getNtpServers();

    public abstract void setNtpServers(List<String> ntpServers);

    public abstract void setNtpConfig(String ntpConf);

    public abstract String getNtpConfig();

    @Persist
    @InitialValue(value = "true")
    public abstract boolean getShowNtpServers();

    public abstract void setShowNtpServers(boolean bool);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        if (getNtpServers() == null) {
            setNtpServers(getTimeManager().getNtpServers());
        }
        if (getNtpConfig() == null) {
            setNtpConfig(getTimeManager().getNtpConfiguration());
        }
    }

    public void submitNtpConfig() {
        if (!getShowNtpServers()) {
            try {
                getTimeManager().setNtpConfiguration(getNtpConfig());
            } catch (UserException ex) {
                TapestryUtils.getValidator(this).record(
                        new ValidatorException(getMessages().getMessage(ex.getMessage())));
            }
        } else {
            try {
                getTimeManager().setNtpServers(getNtpServers());
            } catch (UserException ex) {
                TapestryUtils.getValidator(this).record(
                        new ValidatorException(getMessages().getMessage(ex.getMessage())));
            }
        }
    }
}
