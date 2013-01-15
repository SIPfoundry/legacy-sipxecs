/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.firewall;

import java.util.LinkedList;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.firewall.CallRateLimit;
import org.sipfoundry.sipxconfig.firewall.CallRateManager;
import org.sipfoundry.sipxconfig.firewall.CallRateRule;

public abstract class EditCallRate extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "firewall/EditCallRate";

    @InjectObject("spring:callRateManager")
    public abstract CallRateManager getCallRateManager();

    public abstract Integer getRuleId();

    public abstract void setRuleId(Integer id);

    @Persist
    public abstract CallRateRule getRule();

    public abstract void setRule(CallRateRule rule);

    public abstract String getAction();

    public abstract int getIndex();

    public abstract CallRateLimit getLimit();

    public abstract void setLimit(CallRateLimit limit);

    @Persist(value = "client")
    public abstract List<CallRateLimit> getLimits();

    public abstract void setLimits(List<CallRateLimit> limits);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setSipMethodModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getSipMethodModel();

    public abstract void setIntervalModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getIntervalModel();

    @Override
    public void pageBeginRender(PageEvent event_) {

        if (getLimits() != null) {
            return;
        }

        CallRateRule rule = getRule();

        if (null != rule) {
            return;
        }
        Integer id = getRuleId();
        if (null != id) {
            rule = getCallRateManager().getCallRateRule(id);
        } else {
            rule = new CallRateRule();
        }
        setRule(rule);
        setLimits(rule.getCallRateLimits());
    }

    public void submit() {
        if (!isValid()) {
            // do nothing on errors
            return;
        }
        if ("add".equals(getAction())) {
            List<CallRateLimit> limits = getLimits();
            List<CallRateLimit> newLimits = new LinkedList<CallRateLimit>();
            newLimits.addAll(limits);
            newLimits.add(new CallRateLimit());
            setLimits(newLimits);
        }
    }

    public void commit() {
        if (!isValid()) {
            // do nothing on errors
            return;
        }

        CallRateRule rule = getRule();
        rule.setCallRateLimits(getLimits());
        saveValid(rule);
    }


    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        return !delegate.getHasErrors();
    }

    private void saveValid(CallRateRule rule) {
        getCallRateManager().saveCallRateRule(rule);
        Integer id = rule.getId();
        setRuleId(id);
        setRule(null);
        setLimits(null);
    }

    public void deleteLimit(int index) {
        List<CallRateLimit> limits = getLimits();
        List<CallRateLimit> newLimits = new LinkedList<CallRateLimit>();
        newLimits.addAll(limits);
        newLimits.remove(index);
        getRule().setCallRateLimits(newLimits);
        setLimits(newLimits);
    }

}
