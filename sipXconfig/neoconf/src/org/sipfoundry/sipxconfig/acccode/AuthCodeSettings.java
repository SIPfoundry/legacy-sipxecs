/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.acccode;

import static org.apache.commons.lang.StringUtils.split;
import static org.apache.commons.lang.StringUtils.trim;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AuthCodeSettings extends PersistableSettings implements DeployConfigOnEdit, Replicable {
    public static final String AUTH_CODE_PREFIX = "authcode/SIP_AUTH_CODE_PREFIX";
    public static final String AUTH_CODE_ALIASES = "authcode/SIP_AUTH_CODE_ALIASES";
    private static final Log LOG = LogFactory.getLog(AuthCodeSettings.class);
    private static final String ALIAS_RELATION = "alias";

    /** get the aliases from a space-delimited string */
    public Set<String> getAliasesSet(String aliasesString) {
        LOG.info(String.format("SipxAccCodeService::getAliasesString(): input:%s:", aliasesString));

        Set<String> aliasesSet = new LinkedHashSet<String>(0);

        if (aliasesString != null) {
            String[] aliases = split(aliasesString);
            for (String alias : aliases) {
                aliasesSet.add(trim(alias));
            }
        }
        LOG.info(String.format("SipxAccCodeService::getAliasesString(): retun set :%s:", aliasesSet));
        return aliasesSet;
    }

    public String getAuthCodePrefix() {
        return getSettingValue(AUTH_CODE_PREFIX);
    }

    public void setAuthCodePrefix(String authcodeprefix) {
        setSettingValue(AUTH_CODE_PREFIX, authcodeprefix);
    }

    public String getAuthCodeAliases() {
        return getSettingValue(AUTH_CODE_ALIASES);
    }

    public void setAuthCodeAliases(String aliases) {
        setSettingValue(AUTH_CODE_ALIASES, aliases);
    }

    /** get the aliases from a space-delimited string */
    public Set<String> getAliasesAsSet() {
        String aliasesString = getAuthCodeAliases();
        Set<String> aliasesSet = new LinkedHashSet<String>(0);

        if (aliasesString != null) {
            String[] aliases = split(aliasesString);
            for (String alias : aliases) {
                aliasesSet.add(trim(alias));
            }
        }
        return aliasesSet;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domain) {
        Set<String> aliasesSet = getAliasesAsSet();
        Collection<AliasMapping> mappings = new ArrayList<AliasMapping>(aliasesSet.size());
        // Add alias entry for each extension alias
        // all entries points to the same auth code url
        // sip:AUTH@47.135.162.72:15060;command=auth;
        // see mappingrules.xml
        for (String alias : aliasesSet) {
            // simple alias@bcm2072.com type of identity
            String contact = SipUri.format(getAuthCodePrefix(), domain, false);
            // direct mapping is for testing only
            // contact = getDirectContactUri();
            mappings.add(new AliasMapping(alias, contact, ALIAS_RELATION));
        }
        return mappings;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> dataSets = new HashSet<DataSet>();
        dataSets.add(DataSet.ALIAS);
        return dataSets;
    }

    @Override
    public String getIdentity(String domain) {
        return getAuthCodePrefix() + "@" + domain;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxacccode/sipxacccode.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) AuthCodes.FEATURE, (Feature) DialPlanContext.FEATURE);
    }

    @Override
    public String getBeanId() {
        return "authCodeSettings";
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
