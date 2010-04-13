/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

 /* ProfileContext for NortelFeatureKeyList is created */
 /* when NortelPhone's getFeatureKeyList() method is called */

public class NortelFeatureKeyList extends ProfileContext {
    private final SpeedDial m_speeddial;
    /**
     * @param speeddial provides the access to the SpeedDial buttons
     * configured through the UI (User->SpeedDial on the UI)
     */
    public NortelFeatureKeyList(SpeedDial speeddial) {
        // sets the velocity templates to be used for profile generation
        super(null, "nortel/mac-featurekey.cfg.vm");
        m_speeddial = speeddial;
    }

    public SpeedDial getFeatureKeyList() {
        return m_speeddial;
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        if (m_speeddial != null) {
            Collection<Button> speeddials = m_speeddial.getButtons();
            //speeddials and speeddial objects are used in the velocity templates
            context.put("speeddials", speeddials);
            context.put("speeddial", m_speeddial);
        }
        return context;
    }

}
