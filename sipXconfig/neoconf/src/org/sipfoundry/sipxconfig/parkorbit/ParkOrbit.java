/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class ParkOrbit extends BackgroundMusic implements NamedObject, DeployConfigOnEdit, SystemAuditable,
        Replicable {

    public static final String ALIAS_RELATION = "orbit";
    public static final String ALIAS_UNPARK_RELATION = "unpark";
    private String m_name;
    private String m_extension;
    private String m_description;
    private String m_audioDirectory;
    private AddressManager m_addressManager;
    private Location m_location;
    private Registrar m_registrar;

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    @Deprecated
    public AliasMapping generateAlias(String dnsDomain, String orbitServer) {
        String identity = AliasMapping.createUri(m_extension, dnsDomain);
        String contact = AliasMapping.createUri(m_extension, orbitServer);
        return new AliasMapping(identity, contact, ALIAS_RELATION);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpark/park-orbit.xml");
    }

    public boolean isParkTimeoutEnabled() {
        return (Boolean) getSettingTypedValue("general/enableTimeout");
    }

    public int getParkTimeout() {
        return (Integer) getSettingTypedValue("general/parkTimeout");
    }

    public boolean isMultipleCalls() {
        return (Boolean) getSettingTypedValue("general/multipleCalls");
    }

    public boolean isTransferAllowed() {
        return (Boolean) getSettingTypedValue("general/allowTransfer");
    }

    public String getTransferKey() {
        return (String) getSettingTypedValue("general/transferKey");
    }

    public Location getLocation() {
        return m_location;
    }

    public String getHost() {
        return getLocation().getAddress();
    }

    public void setLocation(Location location) {
        m_location = location;
    }

    public String getUnparkExtension() {
        return getUnparkExtension(true);
    }

    private String getUnparkExtension(boolean escape) {
        String callRetrieveCode = m_registrar.getSettings().getCallRetrieveCode();
        if (escape) {
            return String.format("\\%s%s", callRetrieveCode, m_extension);
        }
        return String.format("%s%s", callRetrieveCode, m_extension);
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FreeswitchFeature.FEATURE);
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.CALL_PARK;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        return ds;
    }

    @Override
    public String getIdentity(String domainName) {
        return SipUri.stripSipPrefix(SipUri.format(null, getExtension(), domainName));
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        if (null != m_addressManager) {
            Address fsAddres = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
            String sipUri = SipUri.format(m_extension, getHost(), fsAddres.getPort());
            String sipUriNoQuote = SipUri.format(m_extension, getHost(), fsAddres.getPort(), false);
            AliasMapping nameMapping = new AliasMapping(m_name, sipUriNoQuote, ALIAS_RELATION);
            AliasMapping lineMapping = new AliasMapping(m_extension, sipUri, ALIAS_RELATION);
            String unparkExtension = getUnparkExtension(false);
            String unparkSipUri = SipUri.format(unparkExtension, getHost(), fsAddres.getPort());
            AliasMapping unparkMapping = new AliasMapping(unparkExtension, unparkSipUri, ALIAS_UNPARK_RELATION);
            mappings.addAll(Arrays.asList(nameMapping, lineMapping, unparkMapping));
        }
        return mappings;
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
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    @Override
    public boolean isReplicationEnabled() {
        return isEnabled();
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }
}
