/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mongo;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

public class MongoServiceConfig extends SipxServiceConfiguration {

    private static final SettingFilter SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {
            boolean firstGeneration = (setting.getParent() == root);
            boolean isLeaf = setting.getValues().isEmpty();
            return firstGeneration && isLeaf;
        }
    };

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext c = super.setupContext(location);
        MongoService service = (MongoService) getService(MongoService.BEAN_ID);
        c.put("settings", SettingUtil.filter(SettingFilter.ALL, service.getSettings().getSetting("mongod.conf")));
        c.put("service", service);
        return c;
    }
}
