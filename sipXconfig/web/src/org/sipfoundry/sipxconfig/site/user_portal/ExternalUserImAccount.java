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

import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.ReportBean;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.im.ExternalImAccount;

public abstract class ExternalUserImAccount extends UserBasePage {

    public static final String PAGE = "user_portal/ExternalUserImAccount";

    private static final String[] PROTOCOLS = {
        "aim", "gtalk", "icq", "irc", "msn", "yahoo"
    };

    public abstract ExternalImAccount getExternalImAccount();

    public abstract void setExternalImAccount(ExternalImAccount account);

    public abstract void setProtocolType(ReportBean type);

    @Persist
    public abstract Integer getExternalImAccountId();

    public abstract void setExternalImAccountId(Integer id);

    public void addExternalUserImAccount(String returnPage) {
        setExternalImAccountId(null);
        setReturnPage(returnPage);
    }

    public void editExternalUserImAccount(Integer accountId, String returnPage) {
        setExternalImAccountId(accountId);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        ExternalImAccount account = getExternalImAccount();
        if (account != null) {
            return;
        }

        Integer accountId = getExternalImAccountId();
        if (accountId != null) {
            account = getCoreContext().getExternalAccountById(accountId);
        } else {
            account = new ExternalImAccount();
            account.setUser(getUser());
        }
        setExternalImAccount(account);
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        ExternalImAccount account = getExternalImAccount();
        getCoreContext().saveExternalAccount(account);
        setExternalImAccountId(account.getId());
    }

    public IPropertySelectionModel getProtocolTypeModel() {
        IPropertySelectionModel model = new StringPropertySelectionModel(PROTOCOLS);
        return new LocalizedOptionModelDecorator(model, getMessages(), "label.");
    }
}
