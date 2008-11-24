/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.Tapestry;
import org.apache.tapestry.TapestryUtils;
import org.apache.tapestry.engine.ServiceEncoder;
import org.apache.tapestry.engine.ServiceEncoding;
import org.apache.tapestry.services.ServiceConstants;

public class ServiceUrlEncoder implements ServiceEncoder {
    private String m_serviceName;
    private String m_url;

    public void encode(ServiceEncoding encoding) {
        if (!isExternalService(encoding)) {
            return;
        }

        String serviceName = encoding.getParameterValue(ServiceConstants.SERVICE);

        if (!serviceName.equals(m_serviceName)) {
            return;
        }

        StringBuilder builder = new StringBuilder(m_url);

        String[] params = encoding.getParameterValues(ServiceConstants.PARAMETER);

        for (String param : params) {
            builder.append("/");
            builder.append(param);
        }

        encoding.setServletPath(builder.toString());

        encoding.setParameterValue(ServiceConstants.SERVICE, null);
        encoding.setParameterValue(ServiceConstants.PAGE, null);
        encoding.setParameterValue(ServiceConstants.PARAMETER, null);
    }

    private boolean isExternalService(ServiceEncoding encoding) {
        String service = encoding.getParameterValue(ServiceConstants.SERVICE);

        return service.equals(Tapestry.EXTERNAL_SERVICE);
    }

    public void decode(ServiceEncoding encoding) {
        String servletPath = encoding.getServletPath();

        if (!servletPath.equals(m_url)) {
            return;
        }

        String pathInfo = encoding.getPathInfo();

        String[] params = TapestryUtils.split(pathInfo.substring(1), '/');

        encoding.setParameterValue(ServiceConstants.SERVICE, m_serviceName);
        encoding.setParameterValues(ServiceConstants.PARAMETER, params);
    }

    public String getServiceName() {
        return m_serviceName;
    }

    public void setServiceName(String serviceName) {
        m_serviceName = serviceName;
    }

    public String getUrl() {
        return m_url;
    }

    public void setUrl(String url) {
        m_url = url;
    }

}
