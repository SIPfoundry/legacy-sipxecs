/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
