/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import java.io.Serializable;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.beanutils.PropertyUtilsBean;
import org.hibernate.collection.PersistentCollection;
import org.hibernate.collection.PersistentList;
import org.hibernate.collection.PersistentMap;
import org.hibernate.collection.PersistentSet;
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.beans.factory.annotation.Required;

/**
 * This class contains the business logic of a typical Add, Modify, Delete
 * actions done to SystemAuditable entities
 */
public class GeneralAuditHandler extends AbstractSystemAuditHandler {

    private static final String PROPERTY_DELIMITATOR = " / ";
    private UserProfileService m_userProfileService;

    /**
     * Handles ConfigChange actions coming from Hibernate: ADDED, MODIFIED,
     * DELETED.
     */
    protected void handleConfigChange(SystemAuditable auditedEntity, ConfigChangeAction configChangeAction,
            String[] properties, Object[] oldValues, Object[] newValues) throws Exception {

        ConfigChange configChange = buildConfigChange(configChangeAction, auditedEntity.getConfigChangeType());
        configChange.setDetails(auditedEntity.getEntityIdentifier());

        if (configChangeAction.equals(ConfigChangeAction.MODIFIED)) {
            handleModifiedConfigChangeProperties(configChange, properties, oldValues, newValues);
        }

        handleAddingOfPersistableSettings(configChange, auditedEntity, configChangeAction);

        getConfigChangeContext().storeConfigChange(configChange);
    }

    /**
     * Handles the case in which a PersistableSetting is created for the first time.
     */
    private void handleAddingOfPersistableSettings(ConfigChange configChange, SystemAuditable auditedEntity,
            ConfigChangeAction configChangeAction) {
        if (configChangeAction.equals(ConfigChangeAction.ADDED)
                && auditedEntity instanceof PersistableSettings) {
            PersistableSettings persistableSettings = (PersistableSettings) auditedEntity;
            List<ConfigChangeValue> configChangeValues = new ArrayList<ConfigChangeValue>();
            for (Setting parentSetting : persistableSettings.getSettings().getValues()) {
                for (Setting setting : parentSetting.getValues()) {
                    String settingValue = setting.getValue();
                    String settingDefaultValue = setting.getDefaultValue();
                    if (settingValue != null && settingDefaultValue != null
                            && !settingValue.equals(settingDefaultValue)) {
                        ConfigChangeValue configChangeValue = new ConfigChangeValue(configChange);
                        configChangeValue.setPropertyName(setting.getPath());
                        configChangeValue.setValueBefore(settingDefaultValue);
                        configChangeValue.setValueAfter(settingValue);
                        configChangeValues.add(configChangeValue);
                    }
                }
            }
            if (!configChangeValues.isEmpty()) {
                configChange.setConfigChangeAction(ConfigChangeAction.MODIFIED);
                configChange.setValues(configChangeValues);
            }
        }
    }

    /**
     * Handles only MODIFIED actions: processes and matches propertyNames with
     * valueBefore and valueAfter
     */
    private void handleModifiedConfigChangeProperties(ConfigChange configChange, String[] properties,
            Object[] oldValues, Object[] newValues) throws Exception {
        for (int counter = 0; counter < properties.length; counter++) {
            Object valueBefore = getValue(oldValues, counter);
            Object valueAfter = getValue(newValues, counter);
            String propertyName = properties[counter];

            if (valueAfter == null && valueBefore == null) {
                continue;
            }

            if (valueAfter != null && valueBefore != null && valueAfter.equals(valueBefore)) {
                continue;
            }

            // special case when ValueStorage is first created
            if (valueAfter instanceof ValueStorage) {
                ValueStorage valueStorage = (ValueStorage) valueAfter;
                Map<?, ?> valueStorageMap = valueStorage.getDatabaseValues();
                for (Object key : valueStorageMap.keySet()) {
                    buildConfigChangeValue(configChange, key.toString(), valueBefore,
                            valueStorageMap.get(key));
                }
            } else {
                buildConfigChangeValue(configChange, propertyName, valueBefore, valueAfter);
            }
        }
    }

    /**
     * Build a ConfigChangeValue from parameters. Method used to avoid duplicate
     * code
     */
    private void buildConfigChangeValue(ConfigChange configChange, String property, Object valueBeforeObject,
            Object valueAfterObject) throws IllegalAccessException, InvocationTargetException,
            NoSuchMethodException, SystemAuditException {

        ConfigChangeValue configChangeValue = new ConfigChangeValue(configChange);
        configChangeValue.setPropertyName(property);
        boolean isValueClassName = false;
        if (valueBeforeObject != null) {
            String valueBefore = getObjectName(valueBeforeObject);
            configChangeValue.setValueBefore(valueBefore);
            if (valueBefore.startsWith(valueBeforeObject.getClass().getName())) {
                isValueClassName = true;
            }
        }
        if (valueAfterObject != null) {
            String valueAfter = getObjectName(valueAfterObject);
            configChangeValue.setValueAfter(valueAfter);
            if (valueAfter.startsWith(valueAfterObject.getClass().getName())) {
                isValueClassName = true;
            }
        }
        if (isValueClassName) {
            buildConfigChangeFromDifferentObjects(valueBeforeObject, valueAfterObject, configChange, property);
        } else {
            configChange.addValue(configChangeValue);
        }
    }

    /**
     * Handles Hibernate collections update calls (both Set and Map versions)
     */
    public void handleCollectionUpdate(Object collection, Serializable key) throws Exception {
        if (collection instanceof PersistentMap) {
            handlePersistentMap(((PersistentMap) collection));
        } else if (collection instanceof PersistentCollection) {
            handlePersistentCollection((PersistentCollection) collection);
        }
    }

    /**
     * Handles PersistentCollection updates
     */
    private void handlePersistentCollection(PersistentCollection collection) throws SystemAuditException {
        Object owner = collection.getOwner();
        if (!(owner instanceof SystemAuditable)) {
            return;
        }
        SystemAuditable systemAuditable = (SystemAuditable) owner;
        if (systemAuditable != null) {
            Iterator<Object> newIterator = null;
            if (collection instanceof PersistentSet) {
                newIterator = ((PersistentSet) collection).iterator();
            } else if (collection instanceof PersistentList) {
                newIterator = ((PersistentList) collection).iterator();
            } else {
                return;
            }

            ConfigChange configChange = buildConfigChange(ConfigChangeAction.MODIFIED,
                    systemAuditable.getConfigChangeType());
            Iterator<Object> oldIterator = null;
            Object storedSnapshot = collection.getStoredSnapshot();
            if (storedSnapshot instanceof Collection) {
                Collection<Object> oldCollection = (Collection<Object>) storedSnapshot;
                oldIterator = oldCollection.iterator();
            } else if (storedSnapshot instanceof Map) {
                Map<Object, Object> oldMap = (Map<Object, Object>) storedSnapshot;
                oldIterator = oldMap.values().iterator();
            } else {
                return;
            }

            ConfigChangeValue configChangeValue = new ConfigChangeValue(configChange);
            StringBuilder valueBeforeBuilder = new StringBuilder();
            StringBuilder valueAfterBuilder = new StringBuilder();
            while (newIterator.hasNext() || oldIterator.hasNext()) {
                handleSetIterator(oldIterator, valueBeforeBuilder, configChangeValue, collection);
                handleSetIterator(newIterator, valueAfterBuilder, configChangeValue, collection);
            }
            String valueBefore = valueBeforeBuilder.substring(0, valueBeforeBuilder.length());
            String valueAfter = valueAfterBuilder.substring(0, valueAfterBuilder.length());
            if (!valueBefore.equals(valueAfter)) {
                configChangeValue.setValueBefore(valueBefore);
                configChangeValue.setValueAfter(valueAfter);
                configChange.addValue(configChangeValue);
                configChange.setDetails(systemAuditable.getEntityIdentifier());
                getConfigChangeContext().storeConfigChange(configChange);
            }
        }
    }

    /**
     * Handles Map updates
     */
    private void handlePersistentMap(PersistentMap collection) throws SystemAuditException,
            IllegalAccessException, InvocationTargetException, NoSuchMethodException {

        SystemAuditable systemAuditable = null;
        Object owner = collection.getOwner();
        if (owner instanceof SystemAuditable) {
            systemAuditable = (SystemAuditable) owner;
        } else {
            Collection<Object> sessionEntities = new ArrayList<Object>(collection.getSession()
                    .getPersistenceContext().getEntitiesByKey().values());
            for (Object parentEntity : sessionEntities) {
                if (parentEntity instanceof SystemAuditable && isChildContainedInParent(owner, parentEntity)) {
                    systemAuditable = (SystemAuditable) parentEntity;
                    break;
                }
            }
        }

        if (systemAuditable != null) {
            Map<Object, Object[]> oldCollection = (Map<Object, Object[]>) collection.getStoredSnapshot();
            Set<Object> newKeysSet = collection.keySet();
            Set<Object> oldKeysSet = oldCollection.keySet();

            ConfigChange configChange = buildConfigChange(ConfigChangeAction.MODIFIED,
                    systemAuditable.getConfigChangeType());

            for (Object valueKey : newKeysSet) {
                handleConfigChangeValue(systemAuditable, configChange, valueKey, oldCollection,
                        collection);
            }
            for (Object valueKey : oldKeysSet) {
                if (!newKeysSet.contains(valueKey)) {
                    handleConfigChangeValue(systemAuditable, configChange, valueKey, oldCollection,
                            collection);
                }
            }
            if (!configChange.getValues().isEmpty()) {
                configChange.setDetails(systemAuditable.getEntityIdentifier());
                getConfigChangeContext().storeConfigChange(configChange);
            }
        }
    }

    /**
     * Utility method to avoid duplicate code
     */
    private void handleSetIterator(Iterator<Object> iterator, StringBuilder stringBuilder,
            ConfigChangeValue configChangeValue, PersistentCollection collection) {
        if (iterator.hasNext()) {
            Object element = iterator.next();
            String elementName = getObjectName(element);
            if (!elementName.isEmpty()) {
                stringBuilder.append(getObjectName(element)).append(PROPERTY_DELIMITATOR);
            }
            String[] roleArray = collection.getRole().split("\\.");
            configChangeValue.setPropertyName(roleArray[roleArray.length - 1]);
        }
    }

    /**
     * Build a ConfigChangeValue from certain parameters. Method used to avoid
     * duplicate code
     */
    private void handleConfigChangeValue(SystemAuditable systemAuditable, ConfigChange configChange,
            Object valueKey, Map<Object, Object[]> oldPersistentMap,
            PersistentMap newPersistentMap) {
        ConfigChangeValue configChangeValue = new ConfigChangeValue();
        configChangeValue.setConfigChange(configChange);
        configChangeValue.setPropertyName(valueKey.toString());
        Object valueBefore = oldPersistentMap.get(valueKey);
        Object valueAfter = newPersistentMap.get(valueKey);
        // if your new value equals the old one, ignore it
        if (valueBefore != null && valueAfter != null
                && valueAfter.equals(valueBefore)) {
            return;
        }
        if (valueBefore != null) {
            configChangeValue.setValueBefore(valueBefore.toString());
        } else {
            configChangeValue.setValueBefore(getSettingDefaultValue(systemAuditable, valueKey));
        }
        if (valueAfter != null) {
            configChangeValue.setValueAfter(valueAfter.toString());
        } else {
            configChangeValue.setValueAfter(getSettingDefaultValue(systemAuditable, valueKey));
        }
        configChange.addValue(configChangeValue);
    }

    /**
     *  Method used to retrieve the default value of a particular setting
     */
    private String getSettingDefaultValue(SystemAuditable systemAuditable, Object valueKey) {
        String defaultValue = null;
        if (valueKey instanceof String) {
            if (systemAuditable instanceof BeanWithSettings) {
                BeanWithSettings beanWithSettings = (BeanWithSettings) systemAuditable;
                defaultValue = beanWithSettings.getSettingDefaultValue((String) valueKey);
            } else if (systemAuditable instanceof Group) {
                Setting groupSettings = ((Group) systemAuditable).inherhitSettingsForEditing(new User());
                defaultValue = groupSettings.getSetting((String) valueKey).getDefaultValue();
            }
        }
        return defaultValue;
    }

    /**
     * This method handle the UserProfile which is only saved in MongoDB
     */
    protected void handleUserProfileConfigChange(User user) throws SystemAuditException,
            IllegalAccessException, InvocationTargetException, NoSuchMethodException {

        UserProfile newUserProfile = user.getUserProfile();
        UserProfile oldUserProfile = m_userProfileService.getUserProfile(user.getId().toString());
        ConfigChange configChange = buildConfigChange(ConfigChangeAction.MODIFIED, user.getConfigChangeType());
        configChange.setDetails(user.getEntityIdentifier());
        buildConfigChangeFromDifferentObjects(oldUserProfile, newUserProfile, configChange, "");
        if (!configChange.getValues().isEmpty()) {
            getConfigChangeContext().storeConfigChange(configChange);
        }
    }

    /**
     * Builds a configChange object from comparing 2 objects (before and after)
     *
     * @param oldObject
     * @param newObject
     * @param configChange
     * @param propertyPrefix
     *            - ex homeAddress + street / must not be null, if you don't
     *            have a prefix, pass an empty string.
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchMethodException
     * @throws SystemAuditException
     */
    private void buildConfigChangeFromDifferentObjects(Object oldObject, Object newObject,
            ConfigChange configChange, String propertyPrefix) throws IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, SystemAuditException {

        Map<?, ?> map = BeanUtils.describe(oldObject);
        PropertyUtilsBean propUtils = new PropertyUtilsBean();

        for (Object propNameObject : map.keySet()) {
            String propertyName = propNameObject.toString();
            Object valueBefore = propUtils.getProperty(oldObject, propertyName);
            Object valueAfter = propUtils.getProperty(newObject, propertyName);
            if (valueBefore != null || valueAfter != null) {
                if (!propertyPrefix.isEmpty()) {
                    propertyName = propertyPrefix + PROPERTY_DELIMITATOR + propertyName;
                }
                if (valueBefore instanceof Address) {
                    buildConfigChangeFromDifferentObjects(valueBefore, valueAfter, configChange, propertyName);
                    continue;
                }
                if (valueBefore != null && valueAfter != null && valueBefore.equals(valueAfter)) {
                    continue;
                }
                buildConfigChangeValue(configChange, propertyName, valueBefore, valueAfter);
            }
        }
    }

    @Required
    public void setUserProfileService(UserProfileService profileService) {
        m_userProfileService = profileService;
    }

}
