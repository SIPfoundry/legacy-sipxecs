/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.components.RenderBlock;

public abstract class ComponentPlugin extends RenderBlock {

    @Parameter(required = true)
    public abstract String getBlockId();

    @Parameter(required = true)
    public abstract Object getEditableEntity();

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        try {
            IPage componentPluginPage = cycle.getPage("plugin/ComponentsHookPage");
            if (componentPluginPage == null) {
                return;
            }
            ((ComponentPluginPage) componentPluginPage).setEditableEntity(getEditableEntity());
            Block block = (Block) componentPluginPage.getComponent(getBlockId());
            if (block == null) {
                return;
            }
            block.renderForComponent(writer, cycle, this);
        } catch (Exception ex) {
            // failed to lookup plug-in page or block inside plug-in page
            return;
        }
    }
}
