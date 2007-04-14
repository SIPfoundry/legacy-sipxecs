/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.clearone;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class ClearoneModel extends PhoneModel {
    public ClearoneModel() {
        super(ClearonePhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public ClearoneModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
