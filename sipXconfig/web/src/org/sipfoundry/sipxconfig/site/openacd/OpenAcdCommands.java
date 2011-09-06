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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.Collection;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.openacd.OpenAcdCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

public abstract class OpenAcdCommands extends BaseComponent {
    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    public abstract OpenAcdCommand getCurrentRow();

    public abstract void setCurrentRow(OpenAcdCommand e);

    public abstract Collection<Integer> getRowsToDelete();

    @InjectPage(EditOpenAcdCommand.PAGE)
    public abstract EditOpenAcdCommand getEditCommandPage();

    @Bean
    public abstract SelectMap getSelections();

    public Set<OpenAcdCommand> getOpenAcdCommands() {
        return getOpenAcdContext().getCommands();
    }

    public abstract void setCommands(Set<OpenAcdCommand> l);

    public IPage editCommand(int id) {
        OpenAcdCommand ext = (OpenAcdCommand) getOpenAcdContext().getExtensionById(id);
        EditOpenAcdCommand page = getEditCommandPage();
        page.setOpenAcdCommandId(ext.getId());
        page.setActions(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public IPage addCommand() {
        EditOpenAcdCommand page = getEditCommandPage();
        page.setOpenAcdCommandId(null);
        page.setActions(null);
        page.setCallback(new PageCallback(this.getPage()));
        return page;
    }

    public void deleteCommands() {
        //delete extensions one by one since we want to trigger onDeleteEvent in order to
        //have delete and mongo write in the same transaction
        //deleteing bulk and publishing delete event will make 2 separate transactions
        for (Integer id : getRowsToDelete()) {
            getOpenAcdContext().deleteExtension(getOpenAcdContext().getExtensionById(id));
        }
    }
}
