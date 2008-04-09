/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

@ComponentClass
public abstract class AutoRefreshForm extends BaseComponent {
    /**
     * Additional actions to be displayed on the left side of refresh button
     */
    @Parameter
    public abstract Block getActionBlock();

    @Persist
    @InitialValue(value = "true")
    public abstract boolean isAuto();

    @Persist
    @InitialValue(value = "30")
    public abstract int getInterval();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Asset(value = "context:/WEB-INF/common/AutoRefreshForm.script")
    public abstract IAsset getAutoRefreshFormScript();

    @Persist
    public abstract int getCurrentInterval();

    public abstract void setCurrentInterval(int interval);

    @EventListener(elements = "auto", events = "onChange")
    public void refresh() {
    }

    public void pageBeginRender(PageEvent event) {
        if (0 == getCurrentInterval()) {
            setCurrentInterval(getInterval());
        }
    }
}
