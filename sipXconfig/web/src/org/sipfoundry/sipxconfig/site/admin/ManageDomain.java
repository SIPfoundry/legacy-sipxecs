/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

/**
 * Edit single domain and it's aliases
 */
public abstract class ManageDomain extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/ManageDomain";

    @InjectObject (value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject (value = "spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InitialValue (value = "ognl:domainManager.domain")
    public abstract Domain getDomain();

    @InjectObject("spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    public abstract int getIndex();

    public abstract void setDomain(Domain domain);

    @Persist (value = "session")
    public abstract List<String> getAliases();

    public abstract void setAliases(List<String> aliases);

    public void pageBeginRender(PageEvent event) {
        List<String> aliases = getAliases();
        if (aliases == null) {
            aliases = new ArrayList<String>();
            aliases.addAll(getDomain().getAliases());
            setAliases(aliases);
        }
    }

    public void removeAlias(int index) {
        getAliases().remove(index);
    }

    public void submit() {
        getAliases().add(StringUtils.EMPTY);
    }

    public void setAlias(String alias) {
        getAliases().set(getIndex(), alias);
    }

    public String getAlias() {
        return getAliases().get(getIndex());
    }

    public void commit() {
        if (!TapestryUtils.isValid(getPage())) {
            return;
        }
        Domain d = getDomain();
        d.getAliases().clear();
        d.getAliases().addAll(getAliases());
        getDomainManager().saveDomain(d);
    }
}
