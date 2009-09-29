/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;


/**
 * One UploadSpecification instance for all types of uploads
 */
public class UploadSpecification extends DeviceDescriptor {

    private boolean m_managed = true;

    public UploadSpecification() {
    }

    public UploadSpecification(String beanId) {
        super(beanId);
    }

    public UploadSpecification(String beanId, String specificationId) {
        super(beanId, specificationId);
    }

    public boolean getManaged() {
        return m_managed;
    }

    public void setManaged(boolean managed) {
        m_managed = managed;
    }

    /**
     * @return getModelId()
     */
    public String getSpecificationId() {
        return getModelId();
    }
}
