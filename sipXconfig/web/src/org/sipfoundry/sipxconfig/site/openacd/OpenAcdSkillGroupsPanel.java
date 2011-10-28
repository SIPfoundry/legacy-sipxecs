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
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkillGroup;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdSkillGroupsPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<OpenAcdSkillGroup> getSkillGroups();

    public abstract void setSkillGroups(List<OpenAcdSkillGroup> skillGroups);

    public abstract Collection getSelectedRows();

    public abstract void setCurrentRow(OpenAcdSkillGroup skillGroup);

    public IPage addSkillGroup(IRequestCycle cycle) {
        EditOpenAcdSkillGroupPage page = (EditOpenAcdSkillGroupPage) cycle.getPage(EditOpenAcdSkillGroupPage.PAGE);
        page.addSkillGroup(getPage().getPageName());
        return page;
    }

    public IPage editSkillGroup(IRequestCycle cycle, Integer skillGroupId) {
        EditOpenAcdSkillGroupPage page = (EditOpenAcdSkillGroupPage) cycle.getPage(EditOpenAcdSkillGroupPage.PAGE);
        page.editSkillGroup(skillGroupId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        List<OpenAcdSkillGroup> groups = getOpenAcdContext().getSkillGroups();
        setSkillGroups(groups);
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        try {
            List<String> groups = getOpenAcdContext().removeSkillGroups(ids);
            if (!groups.isEmpty()) {
                String groupNames = StringUtils.join(groups.iterator(), ", ");
                String errorMessage = getMessages().format("msg.err.skillGroupDeletion", groupNames);
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(errorMessage));
            }
        } catch (UserException ex) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
        }
    }
}
