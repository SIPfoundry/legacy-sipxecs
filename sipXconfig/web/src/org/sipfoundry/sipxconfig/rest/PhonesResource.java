/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.util.Collection;
import java.util.List;

import com.thoughtworks.xstream.XStream;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class PhonesResource extends Resource {

    private static final Log LOG = LogFactory.getLog(PhonesResource.class);

    private PhoneContext m_phoneContext;

    private ModelSource<PhoneModel> m_phoneModelSource;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
    }

    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        PrivatePhoneRepresentation representation = new PrivatePhoneRepresentation(entity);
        Collection<PrivatePhone> newEntries = representation.getObject();

        for (PrivatePhone repphone : newEntries) {
            try {
                LOG.info(String.format("Creating phone %s - %s...", repphone.m_serialNumber, repphone.m_model));
                Phone phone = repphone.getPhone(m_phoneContext, m_phoneModelSource);
                m_phoneContext.storePhone(phone);
                LOG.info(String.format("Created phone ID: %d.", phone.getId()));
            } catch (IllegalArgumentException e) {
                LOG.error("Failed to add phone - possibly invalid model name.", e);
            } catch (UserException e) {
                LOG.error("Failed to add phone - duplicated serial number.", e);
            }
        }
    }

    static class PrivatePhone {
        private final String m_description = null;
        private final String m_serialNumber = null;
        private final String m_model = null;

        public Phone getPhone(PhoneContext phoneContext, ModelSource<PhoneModel> phoneModelSource) {
            PhoneModel model = phoneModelSource.getModel(m_model);
            Phone phone = phoneContext.newPhone(model);
            phone.setDescription(m_description);
            phone.setSerialNumber(m_serialNumber);

            return phone;
        }
    }

    static class PrivatePhoneRepresentation extends XStreamRepresentation<Collection<PrivatePhone>> {
        public PrivatePhoneRepresentation(MediaType mediaType, Collection<PrivatePhone> object) {
            super(mediaType, object);
        }

        public PrivatePhoneRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("phones", List.class);
            xstream.alias("phone", PrivatePhone.class);
        }
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setPhoneModelSource(ModelSource<PhoneModel> phoneModelSource) {
        m_phoneModelSource = phoneModelSource;
    }
}
