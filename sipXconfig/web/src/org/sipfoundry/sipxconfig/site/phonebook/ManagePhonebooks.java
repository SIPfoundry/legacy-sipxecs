/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phonebook.GoogleDomain;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public abstract class ManagePhonebooks extends BasePage implements PageBeginRenderListener {

    @Asset("/images/phonebook.png")
    public abstract IAsset getPhonebookIcon();

    public abstract SelectMap getSelections();

    public abstract PhonebookManager getPhonebookManager();

    public abstract void setGoogleDomainName(String googleDomainName);

    public abstract String getGoogleDomainName();

    public void pageBeginRender(PageEvent event) {
        // load google default domain name
        if (StringUtils.isEmpty(getGoogleDomainName())) {
            setGoogleDomainName(getPhonebookManager().getGoogleDomain().getDomainName());
        }
    }

    public IPage edit(IRequestCycle cycle, Integer phonebookId) {
        EditPhonebook page = (EditPhonebook) cycle.getPage(EditPhonebook.PAGE);
        page.setPhonebookId(phonebookId);
        page.setReturnPage(this);
        return page;
    }

    public IPage addPhonebook(IRequestCycle cycle) {
        EditPhonebook page = (EditPhonebook) cycle.getPage(EditPhonebook.PAGE);
        page.setReturnPage(this);
        return page;
    }

    public void deletePhonebooks() {
        SelectMap selections = getSelections();
        Collection selected = selections.getAllSelected();
        getPhonebookManager().deletePhonebooks(selected);
    }

    public void saveGoogleDomainName() {
        String domainName = getGoogleDomainName();
        if (StringUtils.isEmpty(domainName)) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("error.googleDomain")));
            return;
        }

        GoogleDomain googleDomain = getPhonebookManager().getGoogleDomain();
        googleDomain.setDomainName(getGoogleDomainName());
        getPhonebookManager().saveGoogleDomain(googleDomain);

        TapestryUtils.recordSuccess(getPage(), getMessages().getMessage("msg.googleDomain.success"));
    }
}
