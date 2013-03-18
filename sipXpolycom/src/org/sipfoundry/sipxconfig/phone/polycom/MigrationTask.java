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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Calendar;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.sipfoundry.sipxconfig.setup.SetupManager.Context;

/**
 * Task for migrating Polycom phones to multiple firmware support system. This task, run only once
 * will just generate profiles for Polycom phones and restart them. Restart is scheduled after 1
 * minute to give time to phones to register. See wiki for more details.
 */
public class MigrationTask implements SetupListener {
    private static final Log LOG = LogFactory.getLog(MigrationTask.class);
    private static final String MIGRATION_FLAG = "upgrade-4.6-4.7-phones-migration";
    private PhoneContext m_phoneContext;
    private ProfileManager m_profileManager;

    @Override
    public boolean setup(SetupManager manager) {
    	// only trigger migration of phones when sipxconfig is starting for running
    	// and not during setup or restore phase because phone jobs will take too
    	// long to complete and there's no reason to slow down setup process for that.
        if (manager.getContext() == Context.APP_MAIN && manager.isFalse(MIGRATION_FLAG)) {
            LOG.info("Starting migrating Polycom phones.");
            for (Phone phone : m_phoneContext.loadPhones()) {
                if (phone instanceof PolycomPhone) {
                    Calendar c = Calendar.getInstance();
                    c.roll(Calendar.MINUTE, 1);
                    m_profileManager.generateProfile(phone.getId(), true, c.getTime());
                    LOG.info(String.format(
                            "Generated profiles for %s which will be rebooted in 1 minute from  now.",
                            phone.getSerialNumber()));
                }
            }
        }
        manager.setTrue(MIGRATION_FLAG);
        return true;
    }

    public void setProfileManager(ProfileManager profileManager) {
        m_profileManager = profileManager;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

}
