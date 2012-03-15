/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxivr;

import java.util.Hashtable;

import org.sipfoundry.sipxivr.eslrequest.EslRequestApp;
import org.sipfoundry.sipxivr.eslrequest.EslRequestController;


public abstract class SipxIvrApp implements EslRequestApp {
    private EslRequestController m_controller;

    @Override
    public final void run(Hashtable<String, String> parameters) {
        m_controller.init(parameters);
        run();
    }

    public abstract void run();

    public void setEslRequestController(EslRequestController controller) {
        m_controller = controller;
    }

    public EslRequestController getEslRequestController() {
        return m_controller;
    }

}
