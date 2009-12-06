/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.admin.CertificateDecorator;

public class CertificateSqueezeAdapter implements IPrimaryKeyConverter {
    public Object getPrimaryKey(Object value) {
        CertificateDecorator certificate = (CertificateDecorator) value;
        return certificate.getFileName();
    }

    public Object getValue(Object primaryKey) {
        CertificateDecorator certificate = new CertificateDecorator();
        certificate.setFileName(primaryKey.toString());
        return certificate;
    }
}
