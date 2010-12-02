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
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.openacd.OpenAcdCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

public abstract class OpenAcdCommands extends BaseComponent {
    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    public abstract OpenAcdCommand getCurrentRow();

    public abstract void setCurrentRow(OpenAcdCommand e);

    @Parameter
    public abstract Location getSipxLocation();

    public abstract void setSipxLocation(Location location);

    public abstract Collection<Integer> getRowsToDelete();

    @InjectPage(EditOpenAcdCommand.PAGE)
    public abstract EditOpenAcdCommand getEditCommandPage();

    @Bean
    public abstract SelectMap getSelections();

    public Set<OpenAcdCommand> getOpenAcdCommands() {
        return getOpenAcdContext().getCommands(getSipxLocation());
    }

    public abstract void setCommands(Set<OpenAcdCommand> l);

    public IPage editCommand(int id) {
        OpenAcdCommand ext = (OpenAcdCommand) getOpenAcdContext().getExtensionById(id);
        EditOpenAcdCommand page = getEditCommandPage();
        page.setOpenAcdCommandId(ext.getId());
        page.setSipxLocation(ext.getLocation());
        page.setActions(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public IPage addCommand(Location l) {
        EditOpenAcdCommand page = getEditCommandPage();
        page.setOpenAcdCommandId(null);
        page.setSipxLocation(l);
        page.setActions(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public void deleteCommands() {
        getOpenAcdContext().removeExtensions(getRowsToDelete());
    }
}
