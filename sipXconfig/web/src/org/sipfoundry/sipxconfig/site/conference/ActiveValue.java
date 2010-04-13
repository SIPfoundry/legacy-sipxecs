/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Component;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Any;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.engine.RequestCycle;
import org.apache.tapestry.event.BrowserEvent;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ActiveValue extends BaseComponent {
    private static final String ACTIVE_COUNT_ATTR = "ActiveValue:activeCount";

    @Asset("/images/go.png")
    public abstract IAsset getGoIcon();

    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

    @Asset("context:/WEB-INF/conference/ActiveValue.script")
    public abstract IAsset getScript();

    @Parameter(required = true)
    public abstract int getItemId();

    /**
     * Listener that will be called asynchronously when comonent will try to calculate active
     * count. Listener should store calculated value in the request cycle by calling
     * setActiveCount method.
     */
    @Parameter(required = true)
    public abstract IActionListener getCalculateListener();

    @Parameter(required = false)
    public abstract IActionListener getActivateListener();

    @InitialValue("-1")
    public abstract void setActiveCount(int count);

    public abstract int getActiveCount();

    @Component
    public abstract Any getCell();

    @EventListener(events = "startRendering", targets = "throbber")
    public void retrieveActiveCount(IRequestCycle cycle, BrowserEvent event) {
        // reset the number in case we cannot recalculate it
        setActiveCount(0);
        int id = event.getMethodArguments().getInt(0);
        Object[] params = new Object[] {
            id
        };
        cycle.setListenerParameters(params);
        IActionListener calculateListener = getCalculateListener();
        // listener will set new value of active count in cycle attributes
        calculateListener.actionTriggered(this, cycle);
        Integer newActiveCount = (Integer) cycle.getAttribute(ACTIVE_COUNT_ATTR);
        if (newActiveCount != null) {
            setActiveCount(newActiveCount);
        }
        String cellComponentId = event.getMethodArguments().getString(1);
        // HACK: proper way to do that is...
        // String cellComponentId = getCell().getClientId();
        // but it looks like getCell() always returns the first 'cell' component on this page
        // might be related to https://issues.apache.org/jira/browse/TAPESTRY-2250
        cycle.getResponseBuilder().updateComponent(cellComponentId);
    }

    public Block getBlock() {
        String blockName = "loading";
        int count = getActiveCount();
        if (count >= 0) {
            blockName = getActivateListener() != null ? "link" : "noLink";
        }
        return (Block) getComponent(blockName);
    }

    public String getActiveLabel() {
        return getMessages().format("label.active", getActiveCount());
    }

    public static void setActiveCount(RequestCycle cycle, int activeCount) {
        cycle.setAttribute(ACTIVE_COUNT_ATTR, activeCount);
    }
}
