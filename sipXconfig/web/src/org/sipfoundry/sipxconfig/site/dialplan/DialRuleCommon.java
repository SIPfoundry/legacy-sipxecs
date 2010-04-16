/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

/**
 * EditDialRule
 */
public abstract class DialRuleCommon extends BaseComponent {
    public abstract DialPlanContext getDialPlanContext();

    public abstract GatewayContext getGatewayContext();

    public abstract void setRuleId(Integer ruleId);

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public abstract RuleValidator getValidRule();

    public abstract boolean isRenderGateways();

    public abstract boolean isRuleChanged();

    public abstract void setRuleChanged(boolean ruleChanged);

    public abstract Collection<Integer> getGatewaysToAdd();

    public abstract String getDialRuleHelp();

    boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        try {
            IComponent component = getComponent("common");
            RuleValidator ruleValidator = getValidRule();
            IFormComponent enabled = (IFormComponent) component.getComponent("enabled");
            ruleValidator.validate(enabled, null, getRule());
        } catch (ValidatorException e) {
            delegate.record(e);
        }
        return !delegate.getHasErrors();
    }

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    void saveValid() {
        DialingRule rule = getRule();
        getDialPlanContext().storeRule(rule);
        Integer id = getRule().getId();
        Collection<Integer> gatewaysToAdd = getGatewaysToAdd();
        if (gatewaysToAdd != null) {
            getGatewayContext().addGatewaysToRule(id, gatewaysToAdd);
            setRule(null);
        }
        setRuleId(id);
    }

    public void formSubmit() {
        boolean renderGateways = isRenderGateways();
        boolean ruleChanged = isRuleChanged();
        if (renderGateways && ruleChanged) {
            commit();
        }
    }

    public boolean isAttendantRuleInstance() {
        return getRule() instanceof AttendantRule ? true : false;
    }

    public boolean isDisableEnabledCheckbox() {
        if (getRule() != null && !getRule().isInternal()) {
            if (getRule().getGateways().size() <= 0) {
                return true;
            }
        }
        return false;
    }
}
