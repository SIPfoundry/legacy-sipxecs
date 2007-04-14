/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class HitachiModel extends PhoneModel {
    public HitachiModel() {
        super(HitachiPhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public HitachiModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
