/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.imbot.ImBotManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.Setting;


@ComponentClass
public abstract class MyAssistantComponent extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(MyAssistantComponent.class);

    private static final String FAILURE_MSG = "message.label.requestFailure";

    @Parameter(required = true)
    public abstract User getUser();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject(value = "spring:imBotManager")
    public abstract ImBotManager getImBotManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Setting getImNotificationSettings();

    public abstract void setImNotificationSettings(Setting paSetting);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getImNotificationSettings() == null) {
            setImNotificationSettings(getUser().getSettings().getSetting("im_notification"));
        }
    }

    public void onEnablePA() {
        Future<Object> future = getImBotManager().requestToAddMyAssistantToRoster(getUser().getName());
        try {
            if ((Boolean) future.get(30, TimeUnit.SECONDS)) {
                recordSuccess("message.requestSuccess");
                LOG.debug("Request to Add My Assistant To Roster successfuly published");
            } else {
                recordFailure(FAILURE_MSG);
                LOG.debug("Request to Add My Assistant To Roster failed to get published");
            }
        } catch (Exception e) {
            recordFailure(FAILURE_MSG);
        }
    }

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        getCoreContext().saveUser(getUser());
    }

    private void recordSuccess(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        TapestryUtils.recordSuccess(this, msg);
    }

    private void recordFailure(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        ValidatorException validatorException = new ValidatorException(msg);
        getValidator().record(validatorException);
    }

    public String getSettingsToHide() {
        List<String> names = new LinkedList<String>();
        names.add("call");
        names.add("callFromAnyNumber");
        if (!isVoicemailEnabled()) {
            names.add("leaveMsgBeginIM");
            names.add("leaveMsgEndIM");
        }

        return StringUtils.join(names, ",");
    }

    private boolean isVoicemailEnabled() {
        return (getFeatureManager().isFeatureEnabled(Ivr.FEATURE) ? true : false);
    }
}
