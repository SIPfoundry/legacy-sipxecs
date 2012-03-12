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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.QueueGroupInUseException;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdQueueGroupsPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<OpenAcdQueueGroup> getQueueGroups();

    public abstract void setQueueGroups(List<OpenAcdQueueGroup> queueGroups);

    public abstract Collection getSelectedRows();

    public abstract void setCurrentRow(OpenAcdQueueGroup queueGroup);

    public IPage addQueueGroup(IRequestCycle cycle) {
        EditOpenAcdQueueGroupPage page = (EditOpenAcdQueueGroupPage) cycle.getPage(EditOpenAcdQueueGroupPage.PAGE);
        page.addQueueGroup(getPage().getPageName());
        ((OpenAcdRecipeComponent) page.getComponent(OpenAcdRecipeComponent.NAME)).setRecipeSteps(null);
        return page;
    }

    public IPage editQueueGroup(IRequestCycle cycle, Integer queueGroupId) {
        EditOpenAcdQueueGroupPage page = (EditOpenAcdQueueGroupPage) cycle.getPage(EditOpenAcdQueueGroupPage.PAGE);
        page.editQueueGroup(queueGroupId, getPage().getPageName());
        ((OpenAcdRecipeComponent) page.getComponent(OpenAcdRecipeComponent.NAME)).setRecipeSteps(null);
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        List<OpenAcdQueueGroup> groups = getOpenAcdContext().getQueueGroups();
        setQueueGroups(groups);
    }

    public void delete() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        try {
            List<String> groups = new ArrayList<String>();
            for (Integer id : ids) {
                OpenAcdQueueGroup qgr = getOpenAcdContext().getQueueGroupById(id);
                try {
                    getOpenAcdContext().deleteQueueGroup(qgr);
                } catch (QueueGroupInUseException e) {
                    groups.add(qgr.getName());
                }
            }

            if (!groups.isEmpty()) {
                String groupNames = StringUtils.join(groups.iterator(), ", ");
                String errMessage = getMessages().format("msg.err.queueGroupDeletion", groupNames);
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(errMessage));
            }
        } catch (UserException ex) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
        }
    }
}
