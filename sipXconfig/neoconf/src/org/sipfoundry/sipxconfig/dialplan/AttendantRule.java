/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan;

import static org.sipfoundry.commons.mongo.MongoConstants.ALIAS;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;
import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;
import static org.sipfoundry.sipxconfig.callgroup.AbstractCallSequence.ALIAS_RELATION;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.springframework.beans.factory.annotation.Required;

public class AttendantRule extends DialingRule implements Replicable {

    private static final String SYSTEM_NAME_PREFIX = "aa_";
    private static final String DOUBLE_QUOTE = "\"";
    private static final String LIVE_ATTENDANT_CONTACT = "<sip:%s@%s;sipx-noroute=Voicemail?expires=%d>;q=0.933";
    private static final String LIVE_ATTENDANT_CONTACT_FWD = "<sip:%s@%s;sipx-noroute=Voicemail"
        + ";sipx-userforward=false?expires=%d>;q=0.933";
    private static final String ATTENDANT_CONTACT = "<sip:%s@%s;sipx-noroute=Voicemail?expires=30>;q=0.867";

    private FeatureManager m_featureManager;
    private MediaServer m_mediaServer;
    private ScheduledAttendant m_afterHoursAttendant = new ScheduledAttendant();
    private Holiday m_holidayAttendant = new Holiday();
    private WorkingTime m_workingTimeAttendant = new WorkingTime();
    private String m_attendantAliases;
    private String m_extension;
    private String m_did;
    private boolean m_liveAttendant;
    private String m_liveAttendantExtension;
    private int m_liveAttendantRingFor;
    private boolean m_followUserCallForward;
    private boolean m_liveAttendantEnabled = true;
    private String m_liveAttendantCode;
    private Date m_liveAttendantExpire;

    @Override
    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled() || !m_featureManager.isFeatureEnabled(FreeswitchFeature.FEATURE)) {
            return;
        }
        String[] aliases = AttendantRule.getAttendantAliasesAsArray(m_attendantAliases);
        if (!StringUtils.isEmpty(m_did)) {
            aliases = (String[]) ArrayUtils.add(aliases, m_did);
        }

        m_mediaServer.setLocation(getLocation());
        DialingRule attendantRule = null;
        if (isLiveAttendant()) {
            attendantRule = new MappingRule.Operator(getName(), getDescription(), getSystemName(),
                getAttendantIdentity(), ArrayUtils.EMPTY_STRING_ARRAY, m_mediaServer);
        } else {
            attendantRule = new MappingRule.Operator(getName(), getDescription(), getSystemName(), m_extension,
                aliases, m_mediaServer);
        }
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

    public String getDid() {
        return m_did;
    }

    public void setDid(String did) {
        m_did = did;
    }

    public boolean isLiveAttendant() {
        return m_liveAttendant;
    }

    public void setLiveAttendant(boolean liveAttendant) {
        m_liveAttendant = liveAttendant;
    }

    public String getLiveAttendantExtension() {
        return m_liveAttendantExtension;
    }

    public void setLiveAttendantExtension(String liveAttendantExtension) {
        m_liveAttendantExtension = liveAttendantExtension;
    }

    public int getLiveAttendantRingFor() {
        return m_liveAttendantRingFor;
    }

    public void setLiveAttendantRingFor(int liveAttendantRingFor) {
        m_liveAttendantRingFor = liveAttendantRingFor;
    }

    public boolean isFollowUserCallForward() {
        return m_followUserCallForward;
    }

    public void setFollowUserCallForward(boolean followUserCallForward) {
        m_followUserCallForward = followUserCallForward;
    }

    public String getLiveAttendantCode() {
        return m_liveAttendantCode;
    }

    public void setLiveAttendantCode(String code) {
        m_liveAttendantCode = code;
    }

    public boolean isLiveAttendantEnabled() {
        return m_liveAttendantEnabled;
    }

    public void setLiveAttendantEnabled(boolean enable) {
        m_liveAttendantEnabled = enable;
    }

    public Date getLiveAttendantExpire() {
        return m_liveAttendantExpire;
    }

    public void setLiveAttendantExpire(Date expire) {
        this.m_liveAttendantExpire = expire;
    }

    @Required
    public void setMediaServer(MediaServer mediaServer) {
        m_mediaServer = mediaServer;
    }

    @Required
    public void setFeatureManager(FeatureManager manager) {
        m_featureManager = manager;
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DialPlanContext.FEATURE, (Feature) AutoAttendantManager.FEATURE);
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> dataSets = new HashSet<DataSet>();
        dataSets.add(DataSet.ALIAS);
        dataSets.add(DataSet.USER_FORWARD);

        return dataSets;
    }

    @Override
    public String getIdentity(String domainName) {
        if (isLiveAttendant()) {
            return SipUri.stripSipPrefix(SipUri.format(null, getExtension(), domainName));
        }

        return null;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        String liveContact;
        if (m_followUserCallForward) {
            liveContact = String.format(LIVE_ATTENDANT_CONTACT, getLiveAttendantExtension(), domainName,
                m_liveAttendantRingFor);
        } else {
            liveContact = String.format(LIVE_ATTENDANT_CONTACT_FWD, getLiveAttendantExtension(), domainName,
                m_liveAttendantRingFor);
        }

        if (getSchedule() != null) {
            String validTime = getSchedule().calculateValidTime();
            String scheduleParam = String.format(VALID_TIME_PARAM, DOUBLE_QUOTE + validTime + DOUBLE_QUOTE);
            liveContact += ";" + scheduleParam;
        }

        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        AliasMapping liveAttendantAlias = new AliasMapping(getExtension(), liveContact, ALIAS_RELATION);
        AliasMapping attendantAlias = new AliasMapping(getExtension(), String.format(ATTENDANT_CONTACT,
            getAttendantIdentity(), domainName), ALIAS_RELATION);
        if (m_liveAttendantEnabled) {
            mappings.add(liveAttendantAlias);
        }
        mappings.add(attendantAlias);

        String[] aliases = getAttendantAliasesAsArray(getAttendantAliases());
        if (!StringUtils.isEmpty(m_did)) {
            aliases = (String[]) ArrayUtils.add(aliases, m_did);
        }
        for (String alias : aliases) {
            AliasMapping aliasMapping = new AliasMapping(alias, SipUri.format(getExtension(), domainName, false),
                ALIAS);
            mappings.add(aliasMapping);
        }

        return mappings;
    }

    private String getAttendantIdentity() {
        return String.format("aa_live_%d", getId());
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(UID, m_liveAttendant);
        props.put(CONTACT, SipUri.format(StringUtils.EMPTY, getExtension(), domain));

        return props;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    /**
     * AttendantRule entity must be replicated only when both m_enabled and m_liveAttendant is set to true
     */
    @Override
    public boolean isReplicationEnabled() {
        return isEnabled() && isLiveAttendant();
    }
}
