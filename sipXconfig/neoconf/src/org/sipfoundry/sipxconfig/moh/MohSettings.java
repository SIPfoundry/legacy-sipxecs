/**
 *
 *
 * Copyright (c) 2011 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.moh;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class MohSettings extends PersistableSettings implements Replicable, BeanFactoryAware, DeployConfigOnEdit {
    private static final String MOH_SOURCE = "moh-config/MOH_SOURCE";
    private static final String ALIAS_RELATION = "moh";
    private ListableBeanFactory m_beanFactory;

    public static enum SystemMohSetting {
        FILES_SRC, SOUNDCARD_SRC, NONE;
        public static SystemMohSetting parseSetting(String mohSetting) {
            try {
                return valueOf(mohSetting);
            } catch (IllegalArgumentException e) {
                return FILES_SRC;
            }
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("moh/moh.xml");
    }

    public String getMusicOnHoldSource() {
        return (String) getSettingValue(MOH_SOURCE);
    }

    @Override
    public String getBeanId() {
        return "mohSettings";
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        return ds;
    }

    @Override
    public String getIdentity(String domainName) {
        return SipUri.stripSipPrefix(SipUri.format(null, "~~mh~", domainName));
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>();
        String mohSetting = getMusicOnHoldSource();
        MohAddressFactory moh = getAddressFactory();
        String contact = null;
        switch (SystemMohSetting.parseSetting(mohSetting)) {
        case SOUNDCARD_SRC:
            contact = moh.getPortAudioMohUriMapping();
            break;
        case NONE:
            contact = moh.getNoneMohUriMapping();
            break;
        case FILES_SRC:
        default:
            contact = moh.getLocalFilesMohUriMapping();
            break;
        }

        aliasMappings.add(new AliasMapping(moh.getDefaultUser(), contact, ALIAS_RELATION));
        aliasMappings.add(new AliasMapping(moh.getLocalFilesUser(), moh.getLocalFilesMohUriMapping(),
                ALIAS_RELATION));
        aliasMappings
                .add(new AliasMapping(moh.getPortAudioUser(), moh.getPortAudioMohUriMapping(), ALIAS_RELATION));
        aliasMappings.add(new AliasMapping(moh.getNoneUser(), moh.getNoneMohUriMapping(), ALIAS_RELATION));

        return aliasMappings;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public MohAddressFactory getAddressFactory() {
        return m_beanFactory.getBean(MohAddressFactory.class);
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) MusicOnHoldManager.FEATURE);
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
