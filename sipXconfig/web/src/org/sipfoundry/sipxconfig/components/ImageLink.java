/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.sipfoundry.sipxconfig.components.service.InternalImageService;
import org.sipfoundry.sipxconfig.components.service.InternalImageService.Info;

public abstract class ImageLink extends BaseComponent {
    private String m_path;

    @InjectObject(value = "engine-service:" + InternalImageService.SERVICE_NAME)
    public abstract IEngineService getInternalImageService();

    public String getPath() {
        return m_path;
    }

    public void setPath(String path) {
        m_path = path;
    }

    public String getLink() {
        Info info = new Info(getPath());
        ILink link = getInternalImageService().getLink(false, info);
        return link.getAbsoluteURL();
    }
}
