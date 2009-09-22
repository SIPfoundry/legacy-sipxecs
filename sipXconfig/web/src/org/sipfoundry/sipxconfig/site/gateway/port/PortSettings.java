/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.gateway.port;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class PortSettings extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "gateway/port/PortSettings";

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getPortId();

    public abstract void setPortId(Integer id);

    @Persist
    public abstract void setRuleId(Integer ruleId);

    @Persist
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract FxoPort getPort();

    public abstract void setPort(FxoPort line);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public IPage editSettings(@SuppressWarnings("unused") Integer portId, String section) {
        setParentSettingName(section);
        return this;
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPort() != null) {
            return;
        }

        FxoPort port = getGatewayContext().getPort(getPortId());
        setPort(port);
        Setting root = port.getSettings();
        String parentName = getParentSettingName();
        if (parentName == null) {
            parentName = root.getValues().iterator().next().getPath();
            setParentSettingName(parentName);
        }
        Setting parent = root.getSetting(parentName);
        setParentSetting(parent);
    }

    public void apply() {
        getGatewayContext().storePort(getPort());
    }

    public void ok(IRequestCycle cycle) {
        apply();
        getCallback().performCallback(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
