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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdReleaseCode;

public abstract class EditOpenAcdReleaseCodePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdReleaseCodePage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getOpenAcdReleaseCodeId();

    public abstract void setOpenAcdReleaseCodeId(Integer id);

    public abstract OpenAcdReleaseCode getOpenAcdReleaseCode();

    public abstract void setOpenAcdReleaseCode(OpenAcdReleaseCode code);

    public void addReleaseCode(String returnPage) {
        setOpenAcdReleaseCodeId(null);
        setReturnPage(returnPage);
    }

    public void editReleaseCode(Integer codeId, String returnPage) {
        setOpenAcdReleaseCodeId(codeId);
        setReturnPage(returnPage);
    }

    public IPropertySelectionModel getBiasModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(0, getMessages().getMessage("label.bias.neutral"));
        map.put(1, getMessages().getMessage("label.bias.positive"));
        map.put(-1, getMessages().getMessage("label.bias.negative"));
        return new NamedValuesSelectionModel(map);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getOpenAcdReleaseCodeId() != null) {
            setOpenAcdReleaseCode(getOpenAcdContext().getReleaseCodeById(getOpenAcdReleaseCodeId()));
        } else {
            setOpenAcdReleaseCode(new OpenAcdReleaseCode());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        OpenAcdReleaseCode code = getOpenAcdReleaseCode();
        getOpenAcdContext().saveReleaseCode(code);
        setOpenAcdReleaseCodeId(code.getId());
    }
}
