/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.IOException;
import java.io.OutputStream;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.OutputStreamProfileLocation;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass
public abstract class ProfileLink extends BaseComponent {
    public static final Log LOG = LogFactory.getLog(ProfileLink.class);

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @InjectObject(value = "spring:gatewayContext")
    public abstract GatewayContext getGatewayContext();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @Parameter(required = true)
    public abstract Device getDevice();

    @Parameter(required = true)
    public abstract String getProfile();

    @Parameter(defaultValue = "literal:text/plain")
    public abstract String getContentType();

    @Message(value = "error.generation")
    public abstract String getGenerationErrorMsg();

    public void export(Integer deviceId, boolean isGateway, String profile, String contentType) {
        Device device;
        if (isGateway) {
            device = getGatewayContext().getGateway(deviceId);

        } else {
            device = getPhoneContext().loadPhone(deviceId);
        }
        try {
            OutputStream stream = TapestryUtils.getResponseOutputStream(getResponse(), profile,
                    contentType);
            OutputStreamProfileLocation location = new OutputStreamProfileLocation(stream);
            device.generateProfile(location, profile);
        } catch (IOException e) {
            LOG.error("Generating profile preview.", e);
            throw new UserException(getGenerationErrorMsg(), e.getLocalizedMessage());
        }
    }

    /**
     * Prepares parameters for export method - the order in the result needs to match export
     * parameter lsit.
     */
    public Object[] getParams() {
        Device device = getDevice();
        boolean isGateway = Gateway.class.isAssignableFrom(device.getClass());
        return new Object[] {
            device.getId(), isGateway, getProfile(), getContentType()
        };
    }
}
