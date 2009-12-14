/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.im.ExternalImAccount;
import org.sipfoundry.sipxconfig.site.UserSession;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class XmppUserInfoComponent extends BaseComponent {
    @Parameter(required = true)
    public abstract User getUser();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(ExternalUserImAccount.PAGE)
    public abstract ExternalUserImAccount getExternalUserImAccount();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @Bean
    public abstract SelectMap getSelections();

    public abstract ExternalImAccount getCurrentRow();

    public abstract void setCurrentRow(ExternalImAccount externalAccount);

    public abstract Collection getSelectedRows();

    public IPage addExternalAccount(@SuppressWarnings("unused") IRequestCycle cycle) {
        ExternalUserImAccount page = getExternalUserImAccount();
        page.setUserId(getUser().getId());
        page.addExternalUserImAccount(getPage().getPageName());
        return page;
    }

    public IPage editExternalAccount(@SuppressWarnings("unused") IRequestCycle cycle, Integer accountId) {
        ExternalUserImAccount page = getExternalUserImAccount();
        page.setUserId(getUser().getId());
        page.editExternalUserImAccount(accountId, getPage().getPageName());
        return page;
    }

    public String getLocalizedType() {
        return getMessages().getMessage("label." + getCurrentRow().getType());
    }

    public void deletetExternalAccounts() {
        Collection<Integer> ids = getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        User user = getUser();
        Collection<ExternalImAccount> accounts = user.getExternalImAccounts();
        for (Integer id : ids) {
            accounts.remove(getCoreContext().getExternalAccountById(id));
        }
        getCoreContext().saveUser(getUser());
    }

    public Collection<Integer> getAllSelected() {
        return getSelections().getAllSelected();
    }

    public void save() {
        if (TapestryUtils.isValid(this)) {
            getCoreContext().saveUser(getUser());
        }
    }
}
