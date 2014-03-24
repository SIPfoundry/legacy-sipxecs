package org.sipfoundry.sipxconfig.rest;

import org.apache.commons.lang.StringUtils;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;

/**
 * This is meant to be periodically called by clients in order to keep their web session alive.
 */
public class KeepAliveResource extends UserResource {
    // avoid creating the same object again and again
    private static final StringRepresentation RESPONSE = new StringRepresentation(StringUtils.EMPTY);

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        return RESPONSE;
    }
}