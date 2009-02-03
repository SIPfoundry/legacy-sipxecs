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

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

public interface DialPlanContext extends DataObjectSource, AliasOwner {

    String CONTEXT_BEAN_NAME = "dialPlanContext";

    void clear();

    void storeAutoAttendant(AutoAttendant attendant);

    void deleteAutoAttendant(AutoAttendant attendant);

    AutoAttendant getOperator();

    AutoAttendant getAutoAttendant(Integer id);

    List<AutoAttendant> getAutoAttendants();

    void deleteAutoAttendantsByIds(Collection<Integer> attendantsIds);

    void specialAutoAttendantMode(boolean enabled, AutoAttendant attendant);

    void removeGateways(Collection<Integer> gatewaysIds);

    void storeRule(DialingRule rule);

    void addRule(int position, DialingRule rule);

    List<DialingRule> getRules();

    /**
     * Gets all of the dialing rules using a particular gateway.
     *
     * @param gatewayId The ID of the gateway.
     * @return A List of the DialingRules for that gateway.
     */
    List<DialingRule> getRulesForGateway(Integer gatewayId);

    /**
     * Gets all of the dialing rules that can be added to a particular gateway.
     *
     * @param gatewayId The ID of the gateway
     * @return A List of the DialingRules that can be added to the gateway
     */
    List<DialingRule> getAvailableRules(Integer gatewayId);

    DialingRule getRule(Integer id);

    void deleteRules(Collection<Integer> selectedRows);

    void duplicateRules(Collection<Integer> selectedRows);

    void moveRules(Collection<Integer> selectedRows, int step);

    List<DialingRule> getGenerationRules();

    List<AttendantRule> getAttendantRules();

    DialPlan resetToFactoryDefault();

    DialPlan resetToFactoryDefault(String dialPlanBeanName);

    String[] getDialPlanBeans();

    String getDefaultDialPlanId();

    boolean isDialPlanEmpty();

    String getVoiceMail();

    EmergencyInfo getLikelyEmergencyInfo();

    Group getDefaultAutoAttendantGroup();

    AutoAttendant newAutoAttendantWithDefaultGroup();

    Setting getAttendantSettingModel();
}
