/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

@ComponentClass
public abstract class AjaxToggleButton extends BaseComponent implements PageBeginRenderListener {
    public static final String LINK = "link";
    @Persist
    public abstract String getLinkText();
    public abstract void setLinkText(String text);

    public abstract IComponent getCurrentBlock();
    public abstract void setCurrentBlock(IComponent block);

    @Persist
    @InitialValue(value = "true")
    public abstract boolean isNone();
    public abstract void setNone(boolean none);

    @Parameter(required = true)
    public abstract String getLinkShowText();

    @Parameter(required = true)
    public abstract String getLinkHideText();

    @Parameter(required = true)
    public abstract List<IComponent> getRenderBlocks();

    public void pageBeginRender(PageEvent event) {
        if (getLinkText() == null) {
            setLinkText(getLinkShowText());
        }
    }

    @EventListener(events = "onclick", elements = LINK)
    public void showPanel(IRequestCycle cycle) {
        if (isNone()) {
            setLinkText(getLinkHideText());
            setNone(false);
        } else {
            setLinkText(getLinkShowText());
            setNone(true);
        }
        cycle.getResponseBuilder().updateComponent(LINK);
        cycle.getResponseBuilder().updateComponent("render");
    }
}
