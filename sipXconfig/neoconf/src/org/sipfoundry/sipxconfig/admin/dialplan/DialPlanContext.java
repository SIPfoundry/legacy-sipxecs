/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * DialPlanContext
 */
public interface DialPlanContext extends DataObjectSource, AliasOwner {

    public static final String CONTEXT_BEAN_NAME = "dialPlanContext";

    public abstract void clear();

    public abstract ConfigGenerator generateDialPlan();

    public abstract void activateDialPlan(boolean restartSbcDevices);

    public abstract ConfigGenerator getGenerator();

    public abstract void storeAutoAttendant(AutoAttendant attendant);

    public abstract void deleteAutoAttendant(AutoAttendant attendant, String scriptsDir);

    public abstract AutoAttendant getOperator();

    public abstract AutoAttendant getAutoAttendant(Integer id);

    public abstract List<AutoAttendant> getAutoAttendants();

    public abstract void deleteAutoAttendantsByIds(Collection<Integer> attendantsIds,
            String scriptsDir);

    public abstract void specialAutoAttendantMode(boolean enabled, AutoAttendant attendant);

    public abstract void replicateAutoAttendants();

    public abstract void removeGateways(Collection<Integer> gatewaysIds);

    public void storeRule(DialingRule rule);

    public void addRule(int position, DialingRule rule);

    public List<DialingRule> getRules();

    /**
     * Gets all of the dialing rules using a particular gateway.
     *
     * @param gatewayId The ID of the gateway.
     * @return A List of the DialingRules for that gateway.
     */
    public List<DialingRule> getRulesForGateway(Integer gatewayId);

    /**
     * Gets all of the dialing rules that can be added to a particular gateway.
     *
     * @param gatewayId The ID of the gateway
     * @return A List of the DialingRules that can be added to the gateway
     */
    public List<DialingRule> getAvailableRules(Integer gatewayId);

    public DialingRule getRule(Integer id);

    public void deleteRules(Collection<Integer> selectedRows);

    public void duplicateRules(Collection<Integer> selectedRows);

    public void moveRules(Collection<Integer> selectedRows, int step);

    public List<DialingRule> getGenerationRules();

    public List<AttendantRule> getAttendantRules();

    public DialPlan resetToFactoryDefault();

    public DialPlan resetToFactoryDefault(String dialPlanBeanName);

    public String[] getDialPlanBeans();

    public String getDefaultDialPlanId();

    public boolean isDialPlanEmpty();

    public String getVoiceMail();

    public EmergencyInfo getLikelyEmergencyInfo();

    public Group getDefaultAutoAttendantGroup();

    public AutoAttendant newAutoAttendantWithDefaultGroup();

    public Setting getAttendantSettingModel();
}
