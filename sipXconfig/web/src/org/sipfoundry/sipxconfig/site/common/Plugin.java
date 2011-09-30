/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.site.PluginHook;
import org.sipfoundry.sipxconfig.site.PluginHookManager;

/**
 * Find components on registered Hook implementations that would render for this component
 * rendering given a block id.
 */
@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class Plugin extends BaseComponent implements PageBeginRenderListener {

    public abstract void setHookBlocks(Collection<IComponent> hookBlock);

    public abstract void setHookBlock(IComponent hookBlock);

    @Parameter(required = true)
    public abstract String getBlockId();

    @InjectObject("spring:pluginHookManager")
    public abstract PluginHookManager getPluginHookManager();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        List<IComponent> hookBlocks = new ArrayList<IComponent>();
        Collection<PluginHook> hooks = getPluginHookManager().getHooks();
        for (PluginHook hook : hooks) {
            IPage hookPage = getPage().getRequestCycle().getPage("plugin/" + hook.getHookId());
            String id = hook.getHookId() + getBlockId();
            Map components = hookPage.getComponents();
            IComponent c = (IComponent) components.get(id);
            if (c != null) {
                hookBlocks.add(c);
            }
        }
        setHookBlocks(hookBlocks);
    }
}
