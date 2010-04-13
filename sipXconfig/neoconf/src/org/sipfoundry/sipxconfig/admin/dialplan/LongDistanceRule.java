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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * LongDistanceRule
 */
public class LongDistanceRule extends DialingRule {
    private String m_pstnPrefix = StringUtils.EMPTY;
    private boolean m_pstnPrefixOptional;
    private String m_longDistancePrefix = StringUtils.EMPTY;
    private boolean m_longDistancePrefixOptional;
    private String m_areaCodes = StringUtils.EMPTY;
    private int m_externalLen;
    private String m_permissionName = PermissionName.LONG_DISTANCE_DIALING.getName();

    @Override
    public String[] getPatterns() {
        throw new UnsupportedOperationException("getPatterns not supported for LongDistance rule");
    }

    /**
     * Calculates list of dial patterns for a specified PSTN prefix, long distance prefix and area
     * code.
     *
     * Each dial pattern describes the digit sequence that user dials in order to trigger this
     * rule.
     *
     * @param areaCode single are code for which patterns will be generated
     * @return list of dial patterns objects
     */
    List<DialPattern> calculateDialPatterns(String areaCode) {
        String ac = StringUtils.defaultString(areaCode);
        int acLen = areaCode.length();
        int variableLen = -1;
        if (m_externalLen > 0) {
            // someone specified external len - try to calculate variable len
            variableLen = m_externalLen - acLen;
            if (variableLen < 0) {
                // area code is longer than external lenght - need to cut it
                variableLen = 0;
                ac = areaCode.substring(0, m_externalLen);
            }
        }

        boolean pstnPrefixOptional = m_pstnPrefixOptional || StringUtils.isBlank(m_pstnPrefix);
        boolean longDistancePrefixOptional = m_longDistancePrefixOptional
                || StringUtils.isBlank(m_longDistancePrefix);
        List<DialPattern> patterns = new ArrayList<DialPattern>(4);
        if (StringUtils.isNotBlank(m_pstnPrefix) && StringUtils.isNotBlank(m_longDistancePrefix)) {
            String prefix = m_pstnPrefix + m_longDistancePrefix + ac;
            patterns.add(new DialPattern(prefix, variableLen));
        }
        if (pstnPrefixOptional && StringUtils.isNotBlank(m_longDistancePrefix)) {
            String prefix = m_longDistancePrefix + ac;
            patterns.add(new DialPattern(prefix, variableLen));
        }
        if (StringUtils.isNotBlank(m_pstnPrefix) && longDistancePrefixOptional) {
            String prefix = m_pstnPrefix + ac;
            patterns.add(new DialPattern(prefix, variableLen));
        }
        if (pstnPrefixOptional && longDistancePrefixOptional) {
            patterns.add(new DialPattern(ac, variableLen));
        }
        return patterns;
    }

    /**
     * Calculates the call pattern - the sequence of digits sent to the gateway.
     *
     * @param areaCode single are code for which patterns will be generated
     * @return a single call pattern
     */
    CallPattern calculateCallPattern(String areaCode) {
        StringBuilder prefix = new StringBuilder();
        if (m_longDistancePrefix != null) {
            prefix.append(m_longDistancePrefix);
        }
        if (areaCode != null) {
            prefix.append(areaCode);
        }
        CallPattern callPattern = new CallPattern(prefix.toString(), CallDigits.VARIABLE_DIGITS);
        return callPattern;
    }

    @Override
    public Transform[] getTransforms() {
        throw new UnsupportedOperationException("getTransforms not implemented for LongDistance rule");
    }

    @Override
    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        String[] areaPatterns = DialPattern.getPatternsFromList(m_areaCodes, StringUtils.EMPTY);
        if (0 == areaPatterns.length) {
            CustomDialingRule rule = createCustomRule(StringUtils.EMPTY);
            rule.setDescription(getDescription());
            rules.add(rule);
        } else {
            for (int i = 0; i < areaPatterns.length; i++) {
                String areaCode = areaPatterns[i];
                CustomDialingRule rule = createCustomRule(areaCode);
                rule.setDescription(getDescription());
                rules.add(rule);
            }
        }
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    public boolean isGatewayAware() {
        return true;
    }

    /**
     * Creates a single custom rule that will be used to generate dial and call patterns for a
     * specified areaCode
     *
     * @param areaCode area code inserted in dial and call patterns
     * @return newly created custom rule
     */
    private CustomDialingRule createCustomRule(String areaCode) {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setName(getName());
        rule.setDescription(getDescription());
        rule.setEnabled(isEnabled());
        rule.setGateways(getGateways());
        rule.setCallPattern(calculateCallPattern(areaCode));
        rule.setDialPatterns(calculateDialPatterns(areaCode));
        rule.setPermissionManager(getPermissionManager());
        List<String> permNames = new ArrayList<String>(1);
        if (m_permissionName != null) {
            permNames.add(m_permissionName);
        }
        rule.setPermissionNames(permNames);
        rule.setSchedule(getSchedule());
        return rule;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.LONG_DISTANCE;
    }

    public String getAreaCodes() {
        return m_areaCodes;
    }

    public void setAreaCodes(String areaCodes) {
        m_areaCodes = areaCodes;
    }

    public int getExternalLen() {
        return m_externalLen;
    }

    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
    }

    public String getLongDistancePrefix() {
        return m_longDistancePrefix;
    }

    public void setLongDistancePrefix(String longDistancePrefix) {
        m_longDistancePrefix = longDistancePrefix;
    }

    public String getPstnPrefix() {
        return m_pstnPrefix;
    }

    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
    }

    public Permission getPermission() {
        return getPermission(m_permissionName);
    }

    public void setPermission(Permission permission) {
        if (permission != null) {
            m_permissionName = permission.getName();
        } else {
            m_permissionName = null;
        }
    }

    public void setPermissionName(String permisisonName) {
        m_permissionName = permisisonName;
    }

    public String getPermissionName() {
        return m_permissionName;
    }

    public boolean isPstnPrefixOptional() {
        return m_pstnPrefixOptional;
    }

    public void setPstnPrefixOptional(boolean pstnPrefixOptional) {
        m_pstnPrefixOptional = pstnPrefixOptional;
    }

    public boolean isLongDistancePrefixOptional() {
        return m_longDistancePrefixOptional;
    }

    public void setLongDistancePrefixOptional(boolean longDistancePrefixOptional) {
        m_longDistancePrefixOptional = longDistancePrefixOptional;
    }

}
