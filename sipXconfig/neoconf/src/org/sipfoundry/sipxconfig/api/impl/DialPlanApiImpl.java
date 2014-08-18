/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.api.DialPlanApi;
import org.sipfoundry.sipxconfig.api.model.DialPatternList;
import org.sipfoundry.sipxconfig.api.model.DialingRuleBean;
import org.sipfoundry.sipxconfig.api.model.DialingRuleBean.RuleType;
import org.sipfoundry.sipxconfig.api.model.DialingRuleList;
import org.sipfoundry.sipxconfig.api.model.HolidayBean;
import org.sipfoundry.sipxconfig.api.model.NameList;
import org.sipfoundry.sipxconfig.api.model.WorkingTimeBean;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.dialplan.CallPatternBean;
import org.sipfoundry.sipxconfig.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRuleFactory;
import org.sipfoundry.sipxconfig.dialplan.EmergencyRule;
import org.sipfoundry.sipxconfig.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.dialplan.LongDistanceRule;
import org.sipfoundry.sipxconfig.dialplan.SiteToSiteDialingRule;
import org.sipfoundry.sipxconfig.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.springframework.beans.factory.annotation.Required;

public class DialPlanApiImpl implements DialPlanApi {
    private static final Log LOG = LogFactory.getLog(DialPlanApi.class);

    private DialPlanContext m_dialPlanContext;
    private DialingRuleFactory m_dialingRuleFactory;
    private AutoAttendantManager m_autoAttendantManager;
    private ForwardingContext m_forwardingContext;

    @Override
    public Response getRawRules() {
        Collection<String> rawRules = m_dialingRuleFactory.getBeanIds();
        if (rawRules != null) {
            return Response.ok().entity(NameList.retrieveList(rawRules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getRules() {
        List<DialingRule> rules = m_dialPlanContext.getRules();
        if (rules != null) {
            return Response.ok().entity(DialingRuleList.convertDialingRuleList(rules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getRule(Integer ruleId) {
        DialingRule rule = m_dialPlanContext.getRule(ruleId);
        if (rule != null) {
            DialingRuleBean bean = DialingRuleBean.convertDialingRule(rule);
            return Response.ok().entity(bean).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newRule(DialingRuleBean ruleBean) {
        DialingRule rule = null;
        if (ruleBean.getType() == RuleType.Internal) {
            rule = new InternalRule();
        } else if (ruleBean.getType() == RuleType.Custom) {
            rule = new CustomDialingRule();
        } else if (ruleBean.getType() == RuleType.Long_Distance
            || ruleBean.getType() == RuleType.Restricted || ruleBean.getType() == RuleType.Toll_free) {
            rule = new LongDistanceRule();
        } else if (ruleBean.getType() == RuleType.Emergency) {
            rule = new EmergencyRule();
        } else if (ruleBean.getType() == RuleType.Attendant) {
            rule = new AttendantRule();
        } else if (ruleBean.getType() == RuleType.Site_To_Site) {
            rule = new SiteToSiteDialingRule();
        }
        convertToDialingRule(rule, ruleBean);
        try {
            m_dialPlanContext.storeRule(rule);
        } catch (Exception ex) {
            return Response.status(Status.CONFLICT).entity("User exception during creation "
                    + ex.getClass().getName()).build();
        }
        return Response.ok().entity(rule.getId()).build();
    }

    @Override
    public Response deleteRule(Integer ruleId) {
        DialingRule rule = m_dialPlanContext.getRule(ruleId);

        if (rule != null) {
            m_dialPlanContext.deleteRules(Collections.singleton(ruleId));
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response updateRule(Integer ruleId, DialingRuleBean ruleBean) {
        DialingRule rule = m_dialPlanContext.getRule(ruleId);
        if (rule != null) {
            convertToDialingRule(rule, ruleBean);
            try {
                m_dialPlanContext.storeRule(rule);
            } catch (Exception ex) {
                return Response.status(Status.CONFLICT).entity("User exception during update "
                    + ex.getClass().getName()).build();
            }
            return Response.ok().entity(rule.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private void convertToDialingRule(DialingRule rule, DialingRuleBean ruleBean) {
        rule.setName(ruleBean.getName());
        rule.setDescription(ruleBean.getDescription());
        rule.setEnabled(ruleBean.isEnabled());
        Schedule sch = null;
        try {
            sch = (ruleBean.getScheduleId() == null ? null : m_forwardingContext.getScheduleById(
                ruleBean.getScheduleId()));
        } catch (Exception ex) {
            LOG.debug("The given schedule id is not found");
        }
        rule.setSchedule(sch);
        if (ruleBean.getType() == RuleType.Internal) {
            ((InternalRule) rule).setMediaServerHostname(ruleBean.getMediaServerHostname());
            ((InternalRule) rule).setMediaServerType(ruleBean.getMediaServerType());
        } else if (ruleBean.getType() == RuleType.Custom) {
            ((CustomDialingRule) rule).setDialPatterns(DialPatternList.convertToPatternList(
                ruleBean.getDialPatterns()));
            ((CustomDialingRule) rule).setCallPattern(CallPatternBean.convertToCallPattern(
                ruleBean.getCallPattern()));
        } else if (ruleBean.getType() == RuleType.Long_Distance || ruleBean.getType() == RuleType.Restricted
            || ruleBean.getType() == RuleType.Toll_free) {
            ((LongDistanceRule) rule).setPstnPrefix(ruleBean.getPstnPrefix());
            ((LongDistanceRule) rule).setPstnPrefixOptional(ruleBean.isPstnPrefixOptional());
            ((LongDistanceRule) rule).setLongDistancePrefix(ruleBean.getLongDistancePrefix());
            ((LongDistanceRule) rule).setLongDistancePrefixOptional(ruleBean.isLongDistancePrefixOptional());
            ((LongDistanceRule) rule).setAreaCodes(ruleBean.getAreaCodes());
            ((LongDistanceRule) rule).setExternalLen(ruleBean.getExternalLen());
        } else if (ruleBean.getType() == RuleType.Emergency) {
            ((EmergencyRule) rule).setOptionalPrefix(ruleBean.getOptionalPrefix());
            ((EmergencyRule) rule).setEmergencyNumber(ruleBean.getEmergencyNumber());
        } else if (ruleBean.getType() == RuleType.Attendant) {

            AutoAttendant autoAttendant = retrieveAutoAttendant(ruleBean.getAfterHoursAttendant());

            if (autoAttendant != null) {
                ScheduledAttendant schAttendant = ((AttendantRule) rule).getAfterHoursAttendant();
                schAttendant = (schAttendant == null ? new ScheduledAttendant() : schAttendant);
                schAttendant.setAttendant(autoAttendant);
                schAttendant.setEnabled(ruleBean.isAfterHoursAttendantEnabled());
                ((AttendantRule) rule).setAfterHoursAttendant(schAttendant);
            } else {
                ((AttendantRule) rule).setAfterHoursAttendant(new ScheduledAttendant());
            }
            HolidayBean.convertToHoliday(ruleBean.getHolidayAttendantPeriods(),
                ((AttendantRule) rule).getHolidayAttendant());
            ((AttendantRule) rule).setWorkingTimeAttendant(
                WorkingTimeBean.convertToWorkingTime(ruleBean.getWorkingTimeAttendantPeriods()));
            ((AttendantRule) rule).getHolidayAttendant().setAttendant(
                retrieveAutoAttendant(ruleBean.getHolidayAttendant()));
            ((AttendantRule) rule).getWorkingTimeAttendant().setAttendant(
                retrieveAutoAttendant(ruleBean.getWorkingTimeAttendant()));

            ((AttendantRule) rule).setExtension(ruleBean.getExtension());
            ((AttendantRule) rule).setAttendantAliases(ruleBean.getAttendantAliases());
            ((AttendantRule) rule).setDid(ruleBean.getDid());
            ((AttendantRule) rule).setLiveAttendantEnabled(ruleBean.isEnableLiveAttendant());
        } else if (ruleBean.getType() == RuleType.Site_To_Site) {
            ((SiteToSiteDialingRule) rule).setDialPatterns(
                DialPatternList.convertToPatternList(ruleBean.getDialPatterns()));
            ((SiteToSiteDialingRule) rule).setCallPattern(
                CallPatternBean.convertToCallPattern(ruleBean.getCallPattern()));
        }
    }

    private AutoAttendant retrieveAutoAttendant(String autoAttendantName) {
        if (autoAttendantName == null) {
            return null;
        }
        AutoAttendant autoAttendant = m_autoAttendantManager.getAutoAttendantByName(autoAttendantName);
        if (autoAttendant == null) {
            autoAttendant = new AutoAttendant();
            autoAttendant.setName(autoAttendantName);
            m_autoAttendantManager.storeAutoAttendant(autoAttendant);
        }
        return autoAttendant;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public DialingRuleFactory getDialingRuleFactory() {
        return m_dialingRuleFactory;
    }

    @Required
    public void setDialingRuleFactory(DialingRuleFactory dialingRuleFactory) {
        m_dialingRuleFactory = dialingRuleFactory;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }
}
