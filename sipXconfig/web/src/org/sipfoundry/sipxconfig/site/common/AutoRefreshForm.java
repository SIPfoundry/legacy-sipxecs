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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass
public abstract class AutoRefreshForm extends BaseComponent implements PageBeginRenderListener {

    /**
     * Additional actions to be displayed on the left side of refresh button
     */
    @Parameter
    public abstract Block getActionBlock();

    @Parameter(defaultValue = "ognl:true")
    public abstract boolean getShowRefresh();

    @Parameter(defaultValue = "ognl:{}")
    public abstract List<String> getUpdateComponents();

    @Persist
    @InitialValue(value = "true")
    public abstract boolean isAuto();

    @Parameter(defaultValue = "30")
    public abstract int getInterval();

    @Parameter(defaultValue = "false")
    public abstract boolean getStopOnError();

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

    @EventListener(targets = "interval", events = "onSave")
    public void updateRefreshIntervalOnClient(IRequestCycle cycle) {
        // This will trigger a JavaScript call to update the timer on the client.
        cycle.getResponseBuilder().addStatusMessage(null, "refreshInterval", Integer.toString(getCurrentInterval()));
    }

    public void pageBeginRender(PageEvent event) {
        if (0 == getCurrentInterval()) {
            setCurrentInterval(getInterval());
        }
        //Send the error to the page that displays the component (and error if necessary).
        TapestryUtils.sendValidatorError(getPage(), getValidator());
    }

    public Collection<String> getComponentsToUpdate() {
        List<String> componentsToUpdate = new ArrayList<String>();
        componentsToUpdate.add("refreshedContent");
        componentsToUpdate.addAll(getUpdateComponents());

        return componentsToUpdate;
    }
}
