/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.apache.log4j.Logger;
import org.codehaus.jackson.JsonGenerationException;
import org.codehaus.jackson.map.DeserializationConfig;
import org.codehaus.jackson.map.JsonMappingException;
import org.codehaus.jackson.map.ObjectMapper;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;

/**
 * All conversions involve the same steps. This ensures the same settings are consistently used
 * and deals with handling several exceptions.
 *
 * @param <T> Type of the source/target class
 */
public class JacksonConvert {
    private static final Logger LOG = Logger.getLogger(JacksonConvert.class);
    private static final String ACCEPTED_ENCODING = "UTF-8";

    private JacksonConvert() {
        // utility class, no need to instantiate
    }

    public static <T> StringRepresentation toRepresentation(T bean) throws ResourceException {
        String jsonStr;

        try {
            LOG.debug("Converting (toRepresentation):\t" + bean);
            jsonStr = new ObjectMapper().writeValueAsString(bean);
            LOG.debug("Converted (toRepresentation):\t" + jsonStr);
        } catch (JsonGenerationException e) {
            // it's ok to both log and throw exception here, because thrown ResourceException is
            // not logged anywhere else
            LOG.error("JsonGenerationException", e);
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL);
        } catch (JsonMappingException e) {
            LOG.error("JsonMappingException", e);
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL);
        } catch (IOException e) {
            LOG.error("IOException", e);
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL);
        }

        return new StringRepresentation(jsonStr);
    }

    public static <T> T fromRepresentation(Representation entity, Class<T> clazz) throws ResourceException {
        ObjectMapper mapper = new ObjectMapper();
        mapper.configure(DeserializationConfig.Feature.FAIL_ON_UNKNOWN_PROPERTIES, false);
        T bean;
        try {
            String inStr = IOUtils.toString(entity.getStream(), ACCEPTED_ENCODING);
            LOG.debug("Converting (fromRepresentation):\t" + inStr);
            bean = mapper.readValue(inStr, clazz);
            LOG.debug("Converted (fromRepresentation):\t" + bean);
        } catch (IOException e) {
            LOG.error("IOException:", e);
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL);
        }

        return bean;
    }
}
