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
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.collection.AbstractPersistentCollection;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureListener;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class SystemAuditManagerImpl implements SystemAuditManager, FeatureListener,
        ApplicationListener<ApplicationEvent>, DaoEventListener {

    private static final Log LOG = LogFactory.getLog(SystemAuditManagerImpl.class);
    private static final String LOG_ERROR_MESSAGE = "Exception when processing entry for System Audit: ";

    private GeneralAuditHandler m_generalAuditHandler;
    private FeatureAuditHandler m_featureAuditHandler;
    private LoginLogoutAuditHandler m_loginLogoutAuditHandler;

    @Override
    public void onConfigChangeAction(final Object entity,
            final ConfigChangeAction configChangeAction,
            final String[] properties, final Object[] oldValues,
            final Object[] newValues) {
        if (entity instanceof SystemAuditable) {
            try {
                m_generalAuditHandler.handleConfigChange(
                        (SystemAuditable) entity, configChangeAction,
                        properties, oldValues, newValues);
            } catch (Exception e) {
                LOG.error(LOG_ERROR_MESSAGE, e);
            }
        }
    }

    /**
     * Called before enabling or disabling features.
     * Here we flag which features are marked for enabling or disabling
     */
    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        try {
            m_featureAuditHandler.handlePrecommitFeaturesConfigChange(manager, validator);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        }
    }

    /**
     * Called after enabling or disabling features.
     * Here we know for sure which feature suffered a status change,
     * so we persist a config change action only on those features that are consistent with the precommit request.
     */
    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
        try {
            m_featureAuditHandler.handleFeaturesConfigChange(request);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        }
    }

    /**
     * Called for Login/Logout events
     */
    @Override
    public void onApplicationEvent(ApplicationEvent authEvent) {
        try {
            m_loginLogoutAuditHandler.handleLoginLogoutConfigChange(authEvent);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        }
    }

    @Override
    public void onConfigChangeCollectionUpdate(final Object collection,
            final Serializable key) {
        //need to clearDirty the collection to avoid infinite loops
        if (collection instanceof AbstractPersistentCollection) {
            ((AbstractPersistentCollection) collection).clearDirty();
        }
        try {
            m_generalAuditHandler.handleCollectionUpdate(collection, key);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        } finally {
            //mark it dirty to continue normal processing
            if (collection instanceof AbstractPersistentCollection) {
                ((AbstractPersistentCollection) collection).dirty();
            }
        }
    }

    /**
     * This method only handles UserProfile saves, which don't go through
     * hibernate but are persisted in mongo db
     */
    public void auditUserProfile(User user) {
        if (!user.isNew()) {
            try {
                m_generalAuditHandler.handleUserProfileConfigChange(user);
            } catch (Exception e) {
                LOG.error(LOG_ERROR_MESSAGE, e);
            }
        }
    }

    @Required
    public void setGeneralAuditHandler(GeneralAuditHandler generalAuditHandler) {
        m_generalAuditHandler = generalAuditHandler;
    }

    @Required
    public void setFeatureAuditHandler(FeatureAuditHandler featureAuditHandler) {
        m_featureAuditHandler = featureAuditHandler;
    }

    @Required
    public void setLoginLogoutAuditHandler(LoginLogoutAuditHandler loginLogoutAuditHandler) {
        m_loginLogoutAuditHandler = loginLogoutAuditHandler;
    }

    @Override
    public void auditLicenseUpload(String licenseName) {
        try {
            m_generalAuditHandler.handleLicenseUpload(licenseName);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        }
    }

    @Override
    public void auditServiceRestart(String serverName, List<String> serviceNameList) {
        try {
            m_generalAuditHandler.handleServiceRestart(serverName, serviceNameList);
        } catch (Exception e) {
            LOG.error(LOG_ERROR_MESSAGE, e);
        }
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof Group || entity instanceof Branch) {
            try {
                m_generalAuditHandler.handleConfigChange(
                        (SystemAuditable) entity, ConfigChangeAction.DELETED,
                        null, null, null);
            } catch (Exception e) {
                LOG.error(LOG_ERROR_MESSAGE, e);
            }
        }
    }

    @Override
    public void onSave(Object entity) {
        // Do nothing
    }

}
