/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.AdminNavigation;
import org.sipfoundry.sipxconfig.site.PluginHook;
import org.sipfoundry.sipxconfig.site.PluginHookManager;

/**
 * Find components on registered Hook implementations that would render for this component
 * rendering given a block id.
 */
@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class Plugin extends AdminNavigation implements PageBeginRenderListener {

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
            String featureId = hook.getFeatureId();
            if (featureId == null || isOn(featureId)) {
                String hookId = hook.getHookId();
                IPage hookPage = getPage().getRequestCycle().getPage("plugin/" + hookId);
                String id = hookId + getBlockId();
                Map components = hookPage.getComponents();
                IComponent c = (IComponent) components.get(id);
                if (c != null) {
                    hookBlocks.add(c);
                }
            }
        }
        setHookBlocks(hookBlocks);
    }
}
