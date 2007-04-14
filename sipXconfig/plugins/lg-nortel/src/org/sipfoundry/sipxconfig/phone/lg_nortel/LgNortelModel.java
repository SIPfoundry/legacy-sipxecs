/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class LgNortelModel extends PhoneModel {
    public LgNortelModel() {
        super(LgNortelPhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public LgNortelModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
