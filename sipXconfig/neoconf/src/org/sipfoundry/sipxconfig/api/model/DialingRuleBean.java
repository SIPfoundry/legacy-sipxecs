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
package org.sipfoundry.sipxconfig.api.model;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonProperty;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.CallPatternBean;
import org.sipfoundry.sipxconfig.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.EmergencyRule;
import org.sipfoundry.sipxconfig.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.dialplan.LongDistanceRule;
import org.sipfoundry.sipxconfig.dialplan.SiteToSiteDialingRule;

@XmlRootElement(name = "Rule")
@XmlType(propOrder = {
        "id", "name", "enabled", "type", "description", "scheduleId", "permissionNames", "gatewayAware",
        "authorizationChecked", "internal", "mediaServerHostname", "mediaServerType", "dialPatterns",
        "callPattern", "pstnPrefix", "pstnPrefixOptional", "longDistancePrefix",
        "longDistancePrefixOptional", "areaCodes", "externalLen", "optionalPrefix", "emergencyNumber",
        "afterHoursAttendant", "afterHoursAttendantEnabled", "holidayAttendant", "workingTimeAttendant",
        "holidayAttendantPeriods",
        "workingTimeAttendantPeriods", "extension", "attendantAliases", "did", "enableLiveAttendant"
        })
@JsonPropertyOrder({
    "id", "name", "enabled", "type", "description", "scheduleId", "permissionNames", "gatewayAware",
    "authorizationChecked", "internal", "mediaServerHostname", "mediaServerType", "dialPatterns",
    "callPattern", "pstnPrefix", "pstnPrefixOptional", "longDistancePrefix", "longDistancePrefixOptional",
    "areaCodes", "externalLen", "optionalPrefix", "emergencyNumber", "afterHoursAttendant",
    "afterHoursAttendantEnabled", "holidayAttendant",
    "workingTimeAttendant", "holidayAttendantPeriods", "workingTimeAttendantPeriods", "extension", "attendantAliases",
    "did", "enableLiveAttendant"
    })
public class DialingRuleBean {
    private Integer m_id;
    private RuleType m_type;
    private boolean m_enabled;
    private String m_name;
    private String m_description;
    private String m_emergencyNumber;
    private String m_optionalPrefix;
    private boolean m_gatewayAware;
    private String m_pstnPrefix;
    private boolean m_pstnPrefixOptional;
    private String m_longDistancePrefix;
    private boolean m_longDistancePrefixOptional;
    private String m_areaCodes;
    private int m_externalLen;
    private String m_mediaServerType;
    private String m_mediaServerHostname;
    private String m_did;
    private String m_attendantAliases;
    private String m_extension;
    private boolean m_authorizationChecked;
    private boolean m_internal;
    private Integer m_scheduleId;
    private NameList m_permissionNames;
    private DialPatternList m_dialPatterns;
    private CallPatternBean m_callPattern;
    private String m_afterHoursAttendant;
    private boolean m_afterHoursAttendantEnabled;
    private String m_workingTimeAttendant;
    private String m_holidayAttendant;
    private HolidayBean m_holidayAttendantPeriods;
    private WorkingTimeBean m_workingTimeAttendantPeriods;
    private boolean m_enableLiveAttendant;

    @XmlType(name = "ruleType")
    @XmlEnum
    public enum RuleType {
        @XmlEnumValue(value = "International")
        International,
        @XmlEnumValue(value = "Emergency")
        Emergency,
        @XmlEnumValue(value = "Mapping_Rule")
        Mapping_Rule,
        @XmlEnumValue(value = "Custom")
        Custom,
        @XmlEnumValue(value = "Local")
        Local,
        @XmlEnumValue(value = "Internal")
        Internal,
        @XmlEnumValue(value = "Long_Distance")
        Long_Distance,
        @XmlEnumValue(value = "Restricted")
        Restricted,
        @XmlEnumValue(value = "Toll_free")
        Toll_free,
        @XmlEnumValue(value = "Attendant")
        Attendant,
        @XmlEnumValue(value = "Intercom")
        Intercom,
        @XmlEnumValue(value = "Paging")
        Paging,
        @XmlEnumValue(value = "Site_To_Site")
        Site_To_Site,
        @XmlEnumValue(value = "Authorization_Code")
        Authorization_Code
    }

    public static DialingRuleBean convertDialingRule(DialingRule rule) {
        DialingRuleBean dialingRuleBean = new DialingRuleBean();
        dialingRuleBean.setId(rule.getId());
        dialingRuleBean.setName(rule.getName());
        dialingRuleBean.setDescription(rule.getDescription());
        dialingRuleBean.setEnabled(rule.isEnabled());
        dialingRuleBean.setGatewayAware(rule.isGatewayAware());
        dialingRuleBean.setAuthorizationChecked(rule.isAuthorizationChecked());
        dialingRuleBean.setInternal(rule.isInternal());
        dialingRuleBean.setType(Enum.valueOf(RuleType.class, rule.getType().getName().replace(" ", "_")));
        dialingRuleBean.setScheduleId(rule.getSchedule() != null ? rule.getSchedule().getId() : null);
        dialingRuleBean.setPermissionNames(NameList.retrieveList(rule.getPermissionNames()));
        if (rule instanceof InternalRule) {
            dialingRuleBean.setMediaServerHostname(((InternalRule) rule).getMediaServerHostname());
            dialingRuleBean.setMediaServerType(((InternalRule) rule).getMediaServerType());
        } else if (rule instanceof CustomDialingRule) {
            dialingRuleBean.setDialPatterns(DialPatternList.convertPatternList(
                ((CustomDialingRule) rule).getDialPatterns()));
            dialingRuleBean.setCallPattern(CallPatternBean.convertCallPattern(
                ((CustomDialingRule) rule).getCallPattern()));
        } else if (rule instanceof LongDistanceRule) {
            dialingRuleBean.setPstnPrefix(((LongDistanceRule) rule).getPstnPrefix());
            dialingRuleBean.setPstnPrefixOptional(((LongDistanceRule) rule).isPstnPrefixOptional());
            dialingRuleBean.setLongDistancePrefix(((LongDistanceRule) rule).getLongDistancePrefix());
            dialingRuleBean.setLongDistancePrefixOptional(((LongDistanceRule) rule).isLongDistancePrefixOptional());
            dialingRuleBean.setAreaCodes(((LongDistanceRule) rule).getAreaCodes());
            dialingRuleBean.setExternalLen(((LongDistanceRule) rule).getExternalLen());
        } else if (rule instanceof EmergencyRule) {
            dialingRuleBean.setOptionalPrefix(((EmergencyRule) rule).getOptionalPrefix());
            dialingRuleBean.setEmergencyNumber(((EmergencyRule) rule).getEmergencyNumber());
        } else if (rule instanceof AttendantRule) {
            AutoAttendant attendant = ((AttendantRule) rule).getAfterHoursAttendant().getAttendant();
            dialingRuleBean.setAfterHoursAttendant(attendant != null ? attendant.getName() : null);
            dialingRuleBean.setAfterHoursAttendantEnabled(((AttendantRule) rule).getAfterHoursAttendant().isEnabled());
            attendant = ((AttendantRule) rule).getHolidayAttendant().getAttendant();
            dialingRuleBean.setHolidayAttendant(attendant != null ? attendant.getName() : null);
            attendant = ((AttendantRule) rule).getWorkingTimeAttendant().getAttendant();
            dialingRuleBean.setWorkingTimeAttendant(attendant != null ? attendant.getName() : null);
            attendant = ((AttendantRule) rule).getHolidayAttendant().getAttendant();
            dialingRuleBean.setHolidayAttendantPeriods(HolidayBean.convertHolidayBean(
                ((AttendantRule) rule).getHolidayAttendant()));
            dialingRuleBean.setWorkingTimeAttendantPeriods(WorkingTimeBean.convertWorkingTimeBean(
                ((AttendantRule) rule).getWorkingTimeAttendant()));
            dialingRuleBean.setExtension(((AttendantRule) rule).getExtension());
            dialingRuleBean.setAttendantAliases(((AttendantRule) rule).getAttendantAliases());
            dialingRuleBean.setDid(((AttendantRule) rule).getDid());
            dialingRuleBean.setEnableLiveAttendant(((AttendantRule) rule).isLiveAttendantEnabled());
        } else if (rule instanceof SiteToSiteDialingRule) {
            dialingRuleBean.setDialPatterns(DialPatternList.convertPatternList(
                ((SiteToSiteDialingRule) rule).getDialPatterns()));
            dialingRuleBean.setCallPattern(CallPatternBean.convertCallPattern(
                ((SiteToSiteDialingRule) rule).getCallPattern()));
        }
        return dialingRuleBean;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public boolean isEnabled() {
        return m_enabled;
    }
    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }
    public String getDescription() {
        return m_description;
    }
    public void setDescription(String description) {
        m_description = description;
    }

    public RuleType getType() {
        return m_type;
    }

    public void setType(RuleType type) {
        m_type = type;
    }

    public String getEmergencyNumber() {
        return m_emergencyNumber;
    }
    public void setEmergencyNumber(String emergencyNumber) {
        m_emergencyNumber = emergencyNumber;
    }
    public String getOptionalPrefix() {
        return m_optionalPrefix;
    }
    public void setOptionalPrefix(String optionalPrefix) {
        m_optionalPrefix = optionalPrefix;
    }
    public boolean isGatewayAware() {
        return m_gatewayAware;
    }
    public void setGatewayAware(boolean gatewayAware) {
        m_gatewayAware = gatewayAware;
    }
    public String getPstnPrefix() {
        return m_pstnPrefix;
    }
    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
    }
    public int getExternalLen() {
        return m_externalLen;
    }
    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
    }
    public String getMediaServerType() {
        return m_mediaServerType;
    }
    public void setMediaServerType(String mediaServerType) {
        m_mediaServerType = mediaServerType;
    }
    public String getMediaServerHostname() {
        return m_mediaServerHostname;
    }
    public void setMediaServerHostname(String mediaServerHostname) {
        m_mediaServerHostname = mediaServerHostname;
    }
    public String getDid() {
        return m_did;
    }
    public void setDid(String did) {
        m_did = did;
    }

    public String getAttendantAliases() {
        return m_attendantAliases;
    }
    public void setAttendantAliases(String attendantAliases) {
        m_attendantAliases = attendantAliases;
    }
    public String getExtension() {
        return m_extension;
    }
    public void setExtension(String extension) {
        m_extension = extension;
    }

    public boolean isAuthorizationChecked() {
        return m_authorizationChecked;
    }

    public void setAuthorizationChecked(boolean authorizationChecked) {
        m_authorizationChecked = authorizationChecked;
    }

    public boolean isInternal() {
        return m_internal;
    }

    public void setInternal(boolean internal) {
        m_internal = internal;
    }

    public Integer getScheduleId() {
        return m_scheduleId;
    }

    public void setScheduleId(Integer scheduleId) {
        m_scheduleId = scheduleId;
    }

    @XmlElement(name = "permissions")
    @JsonProperty(value = "permissions")
    public NameList getPermissionNames() {
        return m_permissionNames;
    }

    public void setPermissionNames(NameList permissionNames) {
        m_permissionNames = permissionNames;
    }

    public DialPatternList getDialPatterns() {
        return m_dialPatterns;
    }

    public void setDialPatterns(DialPatternList dialPatterns) {
        m_dialPatterns = dialPatterns;
    }

    public CallPatternBean getCallPattern() {
        return m_callPattern;
    }

    public void setCallPattern(CallPatternBean callPattern) {
        m_callPattern = callPattern;
    }

    public String getLongDistancePrefix() {
        return m_longDistancePrefix;
    }

    public void setLongDistancePrefix(String longDistancePrefix) {
        m_longDistancePrefix = longDistancePrefix;
    }

    public boolean isLongDistancePrefixOptional() {
        return m_longDistancePrefixOptional;
    }

    public void setLongDistancePrefixOptional(boolean longDistancePrefixOptional) {
        m_longDistancePrefixOptional = longDistancePrefixOptional;
    }

    public String getAreaCodes() {
        return m_areaCodes;
    }

    public void setAreaCodes(String areaCodes) {
        m_areaCodes = areaCodes;
    }

    public boolean isPstnPrefixOptional() {
        return m_pstnPrefixOptional;
    }

    public void setPstnPrefixOptional(boolean pstnPrefixOptional) {
        m_pstnPrefixOptional = pstnPrefixOptional;
    }

    public String getAfterHoursAttendant() {
        return m_afterHoursAttendant;
    }

    public void setAfterHoursAttendant(String afterHoursAttendant) {
        m_afterHoursAttendant = afterHoursAttendant;
    }

    public boolean isAfterHoursAttendantEnabled() {
        return m_afterHoursAttendantEnabled;
    }

    public void setAfterHoursAttendantEnabled(boolean afterHoursAttendantEnabled) {
        m_afterHoursAttendantEnabled = afterHoursAttendantEnabled;
    }

    public HolidayBean getHolidayAttendantPeriods() {
        return m_holidayAttendantPeriods;
    }

    public void setHolidayAttendantPeriods(HolidayBean holidayAttendantPeriods) {
        m_holidayAttendantPeriods = holidayAttendantPeriods;
    }

    public WorkingTimeBean getWorkingTimeAttendantPeriods() {
        return m_workingTimeAttendantPeriods;
    }

    public void setWorkingTimeAttendantPeriods(WorkingTimeBean workingTimeAttendantPeriods) {
        m_workingTimeAttendantPeriods = workingTimeAttendantPeriods;
    }

    public boolean isEnableLiveAttendant() {
        return m_enableLiveAttendant;
    }

    public void setEnableLiveAttendant(boolean enableLiveAttendant) {
        m_enableLiveAttendant = enableLiveAttendant;
    }

    public String getWorkingTimeAttendant() {
        return m_workingTimeAttendant;
    }

    public void setWorkingTimeAttendant(String workingTimeAttendant) {
        m_workingTimeAttendant = workingTimeAttendant;
    }

    public String getHolidayAttendant() {
        return m_holidayAttendant;
    }

    public void setHolidayAttendant(String holidayAttendant) {
        m_holidayAttendant = holidayAttendant;
    }
}
