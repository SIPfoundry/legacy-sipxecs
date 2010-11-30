/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;

public abstract class OpenAcdLines extends BaseComponent {
    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    public abstract OpenAcdExtension getCurrentRow();

    public abstract void setCurrentRow(OpenAcdExtension e);

    @Parameter
    public abstract Location getSipxLocation();

    public abstract void setSipxLocation(Location location);

    public abstract Collection<Integer> getRowsToDelete();

    @InjectPage(EditOpenAcdLine.PAGE)
    public abstract EditOpenAcdLine getEditLinePage();

    @Bean
    public abstract SelectMap getSelections();

    public Set<OpenAcdExtension> getOpenAcdLines() {
        return getOpenAcdContext().getLines(getSipxLocation());
    }

    public abstract void setLines(Set<OpenAcdExtension> l);

    public String getQueue() {
        List<FreeswitchAction> actions = getCurrentRow().getLineActions();
        for (FreeswitchAction action : actions) {
            String data = action.getData();
            if (StringUtils.contains(data, OpenAcdExtension.Q)) {
                return StringUtils.removeStart(data, OpenAcdExtension.Q);
            }
        }
        return "";
    }

    public IPage editLine(int id) {
        OpenAcdExtension ext = getOpenAcdContext().getExtensionById(id);
        EditOpenAcdLine page = getEditLinePage();
        page.setOpenAcdLineId(ext.getId());
        page.setSipxLocation(ext.getLocation());
        page.setActions(null);
        page.setWelcomeMessage(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public IPage addLine(Location l) {
        EditOpenAcdLine page = getEditLinePage();
        page.setOpenAcdLineId(null);
        page.setSipxLocation(l);
        page.setActions(null);
        page.setWelcomeMessage(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public void deleteLines() {
        getOpenAcdContext().removeExtensions(getRowsToDelete());
    }
}
