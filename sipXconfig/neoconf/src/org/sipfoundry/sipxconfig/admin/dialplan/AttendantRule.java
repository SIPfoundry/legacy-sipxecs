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

import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.springframework.beans.factory.annotation.Required;

public class AttendantRule extends DialingRule {
    private static final String SYSTEM_NAME_PREFIX = "aa_";

    private ScheduledAttendant m_afterHoursAttendant = new ScheduledAttendant();
    private Holiday m_holidayAttendant = new Holiday();
    private WorkingTime m_workingTimeAttendant = new WorkingTime();
    private String m_attendantAliases;
    private String m_extension;

    private MediaServer m_mediaServer;

    @Override
    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        String[] aliases = AttendantRule.getAttendantAliasesAsArray(m_attendantAliases);
        DialingRule attendantRule = new MappingRule.Operator(getName(), getDescription(), getSystemName(),
                m_extension, aliases, m_mediaServer);
        rules.add(attendantRule);
    }

    @Override
    protected Object clone() throws CloneNotSupportedException {
        AttendantRule ar = (AttendantRule) super.clone();
        ar.m_afterHoursAttendant = (ScheduledAttendant) m_afterHoursAttendant.clone();
        ar.m_workingTimeAttendant = (WorkingTime) m_workingTimeAttendant.clone();
        ar.m_holidayAttendant = (Holiday) m_holidayAttendant.clone();
        return ar;
    }

    /**
     * This is the name passed to the mediaserver cgi to locate the correct auto attendant. It's
     * invalid until saved to database.
     */
    public String getSystemName() {
        return SYSTEM_NAME_PREFIX + getId();
    }

    @Override
    public String[] getPatterns() {
        return null;
    }

    @Override
    public Transform[] getTransforms() {
        return null;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.ATTENDANT;
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }

    public ScheduledAttendant getAfterHoursAttendant() {
        return m_afterHoursAttendant;
    }

    public void setAfterHoursAttendant(ScheduledAttendant afterHoursAttendant) {
        m_afterHoursAttendant = afterHoursAttendant;
    }

    public Holiday getHolidayAttendant() {
        return m_holidayAttendant;
    }

    public void setHolidayAttendant(Holiday holidayAttendant) {
        m_holidayAttendant = holidayAttendant;
    }

    public WorkingTime getWorkingTimeAttendant() {
        return m_workingTimeAttendant;
    }

    public void setWorkingTimeAttendant(WorkingTime workingTimeAttendant) {
        m_workingTimeAttendant = workingTimeAttendant;
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

    @Required
    public void setMediaServer(MediaServer mediaServer) {
        m_mediaServer = mediaServer;
    }

    /**
     * Check if the attendant in question is referenced by this rule
     *
     * @param attendant
     * @return true if any references have been found false otherwise
     */
    public boolean checkAttendant(AutoAttendant attendant) {
        boolean result = m_afterHoursAttendant.checkAttendant(attendant);
        result |= m_workingTimeAttendant.checkAttendant(attendant);
        result |= m_holidayAttendant.checkAttendant(attendant);
        return result;
    }

    public static String[] getAttendantAliasesAsArray(String aliasesString) {
        if (aliasesString == null) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        return StringUtils.split(aliasesString);
    }
}
