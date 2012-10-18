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
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.feature.FeatureManager;

public class OpenAcdReplicationProviderImpl implements OpenAcdReplicationProvider {
    private OpenAcdContext m_openAcdContext;
    private FeatureManager m_featureManager;

    /* (non-Javadoc)
     * @see org.sipfoundry.sipxconfig.openacd.OpenAcdReplicationProvider#getReplicables()
     */
    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        if (!m_featureManager.isFeatureEnabled(OpenAcdContext.FEATURE)) {
            return replicables;
        }
        for (OpenAcdExtension ext : m_openAcdContext.getFreeswitchExtensions()) {
            if (ext.isEnabled()) {
                replicables.add(ext);
            }
        }
        replicables.addAll(m_openAcdContext.getClients());
        replicables.addAll(m_openAcdContext.getQueueGroups());
        replicables.addAll(m_openAcdContext.getQueues());
        replicables.addAll(m_openAcdContext.getAgentGroups());
        replicables.addAll(m_openAcdContext.getAgents());
        replicables.addAll(m_openAcdContext.getReleaseCodes());
        replicables.addAll(m_openAcdContext.getSkills());
        OpenAcdSettings settings = m_openAcdContext.getSettings();
        replicables.add(new FreeswitchMediaCommand(settings.isEnabled(), settings.getCNode(), settings
                .getDialString()));
        replicables.add(new OpenAcdAgentConfigCommand(settings.getDialPlanListener()));
        replicables.add(new OpenAcdLogConfigCommand(settings.getLogLevel(), settings.getLogDir()
                + OpenAcdContext.OPENACD_LOG));
        replicables.add(new OpenAcdAgentWebConfigCommand(settings.isAgentWebUiEnabled(), settings.getAgentWebUiPort(),
                settings.isAgentWebUiSSlEnabled(), settings.getAgentWebUiSSlPort()));
        return replicables;
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }
}
