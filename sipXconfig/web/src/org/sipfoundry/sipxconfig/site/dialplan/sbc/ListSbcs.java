/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.AuxSbc;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ListSbcs extends BaseComponent {
    @InjectObject(value = "spring:sbcManager")
    public abstract SbcManager getSbcManager();

    @InjectPage(value = EditSbc.PAGE)
    public abstract EditSbc getEditSbcPage();

    @Parameter
    public abstract SipxValidationDelegate getValidator();

    public abstract List<AuxSbc> getSbcs();

    public abstract void setSbcs(List<AuxSbc> sbcs);

    public abstract AuxSbc getCurrentRow();

    @Bean
    public abstract SelectMap getSelections();

    public abstract Collection<Integer> getRowsToDelete();

    @Parameter(required = false)
    public abstract void setEnforceInternetCallingSupport(boolean enforce);

    public abstract boolean getEnforceInternetCallingSupport();

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }

    public IPage edit(int sbcId) {
        EditSbc editSbcPage = getEditSbcPage();
        editSbcPage.setSbcId(sbcId);
        editSbcPage.setReturnPage(getPage());
        editSbcPage.setEnforceInternetCallingSupport(getEnforceInternetCallingSupport());
        return editSbcPage;
    }

    public IPage add() {
        EditSbc editSbcPage = getEditSbcPage();
        editSbcPage.setSbcId(null);
        editSbcPage.setReturnPage(getPage());
        editSbcPage.setEnforceInternetCallingSupport(getEnforceInternetCallingSupport());
        return editSbcPage;
    }

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getSbcs() == null) {
            setSbcs(getSbcManager().loadAuxSbcs());
        }
    }

    @Override
    protected void cleanupAfterRender(IRequestCycle cycle) {
        if (!TapestryUtils.isRewinding(cycle, this)) {
            return;
        }
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getSbcManager().removeSbcs(selectedRows);
            setSbcs(null);
        }
    }
}
