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
import java.util.HashMap;
import java.util.Map;

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
import org.sipfoundry.sipxconfig.openacd.OpenAcdReleaseCode;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdReleaseCodesPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setCurrentRow(OpenAcdReleaseCode code);

    public abstract Collection getSelectedRows();

    public IPage addReleaseCode(IRequestCycle cycle) {
        EditOpenAcdReleaseCodePage page = (EditOpenAcdReleaseCodePage) cycle
                .getPage(EditOpenAcdReleaseCodePage.PAGE);
        page.addReleaseCode(getPage().getPageName());
        return page;
    }

    public IPage editReleaseCode(IRequestCycle cycle, Integer codeId) {
        EditOpenAcdReleaseCodePage page = (EditOpenAcdReleaseCodePage) cycle
                .getPage(EditOpenAcdReleaseCodePage.PAGE);
        page.editReleaseCode(codeId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {

    }

    public Map<Integer, String> biasLabel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(0, getMessages().getMessage("label.bias.neutral"));
        map.put(1, getMessages().getMessage("label.bias.positive"));
        map.put(-1, getMessages().getMessage("label.bias.negative"));
        return map;
    }

    public String translateValue(int value) {
        return biasLabel().get(value);
    }

    public void delete() {
        Collection codeIds = getSelections().getAllSelected();
        if (codeIds.isEmpty()) {
            return;
        }
        try {
            getOpenAcdContext().removeReleaseCodes(codeIds);
        } catch (UserException ex) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
        }
    }
}
