/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.Tapestry;
import org.apache.tapestry.engine.ServiceEncoder;
import org.apache.tapestry.engine.ServiceEncoding;
import org.apache.tapestry.services.ServiceConstants;

/**
 * Map human friendly urls to/from tapestry urls
 *
 *   /mailbox/userid/folderid                    - manage voicemails page
 *   /mailbox/userid/folderid/messageid          - plays a voicemail
 *   /mailbox/userid/folderid/messageid/delete   - deletes a voicemail
 *   /mailbox/userid/folderid/messageid/save     - saves a voicemail
 */
public class MailboxPageEncoder implements ServiceEncoder {
    private String m_url;

    public void setUrl(String url) {
        m_url = url;
    }

    public void decode(ServiceEncoding encoding) {
        String servletPath = encoding.getServletPath();

        if (!servletPath.equals(m_url)) {
            return;
        }

        encoding.setParameterValue(ServiceConstants.SERVICE, Tapestry.EXTERNAL_SERVICE);
        encoding.setParameterValue(ServiceConstants.PAGE, ManageVoicemail.PAGE);
        // "S" is so tapestry keeps it a string
        encoding.setParameterValue(ServiceConstants.PARAMETER, "S" + encoding.getPathInfo());
    }

    public void encode(ServiceEncoding encoding) {
        if (!isExternalService(encoding)) {
            return;
        }

        String pageName = encoding.getParameterValue(ServiceConstants.PAGE);
        if (!pageName.equals(ManageVoicemail.PAGE)) {
            return;
        }

        StringBuilder builder = new StringBuilder(m_url).append('/');
        String[] params = encoding.getParameterValues(ServiceConstants.PARAMETER);
        builder.append(StringUtils.join(params, "/"));
        encoding.setServletPath(builder.toString());

        encoding.setParameterValue(ServiceConstants.SERVICE, null);
        encoding.setParameterValue(ServiceConstants.PAGE, null);
        encoding.setParameterValue(ServiceConstants.PARAMETER, null);
    }

    private boolean isExternalService(ServiceEncoding encoding) {
        String service = encoding.getParameterValue(ServiceConstants.SERVICE);
        return service.equals(Tapestry.EXTERNAL_SERVICE);
    }
}
