/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

public interface Model {
    /**
     * Called to set modelId to spring bean ID when loading model form bean factory
     *
     * @param modelId - bean ID representing this model
     */
    void setModelId(String modelId);
}
