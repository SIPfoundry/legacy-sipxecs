/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.util.List;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.converters.SingleValueConverter;
import com.thoughtworks.xstream.converters.UnmarshallingContext;
import com.thoughtworks.xstream.converters.collections.CollectionConverter;
import com.thoughtworks.xstream.io.HierarchicalStreamReader;
import com.thoughtworks.xstream.mapper.Mapper;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractCallSequence;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class ForwardingResource extends UserResource {
    private ForwardingContext m_forwardingContext;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(getUser());
        CallSequence reprCallSequence = (CallSequence) callSequence.duplicate();
        return new CallSequenceRepresentation(variant.getMediaType(), reprCallSequence);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        CallSequenceRepresentation representation = new CallSequenceRepresentation(entity);
        CallSequence newCallSequence = representation.getObject();
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(getUser());
        final List<AbstractRing> rings = newCallSequence.getRings();
        callSequence.replaceRings(rings);
        callSequence.setWithVoicemail(newCallSequence.isWithVoicemail());
        m_forwardingContext.saveCallSequence(callSequence);
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    private static class RingTypeConverter implements SingleValueConverter {
        public boolean canConvert(Class type) {
            return type.equals(Type.class);
        }

        public Object fromString(String str) {
            return Type.getEnum(str);
        }

        public String toString(Object obj) {
            Type type = (Type) obj;
            return type.getName();
        }
    }

    static class RingsConverter extends CollectionConverter {
        public RingsConverter(Mapper mapper) {
            super(mapper);
        }

        @Override
        protected Object readItem(HierarchicalStreamReader reader, UnmarshallingContext context, Object current) {
            return context.convertAnother(current, Ring.class);
        }
    }

    static class CallSequenceRepresentation extends XStreamRepresentation<CallSequence> {

        public CallSequenceRepresentation(MediaType mediaType, CallSequence object) {
            super(mediaType, object);
        }

        public CallSequenceRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, "m_id");
            xstream.alias("call-sequence", CallSequence.class);
            xstream.omitField(CallSequence.class, "m_user");
            xstream.omitField(CallSequence.class, "m_cfwdTime");
            xstream.alias("ring", Ring.class);
            xstream.omitField(Ring.class, "m_schedule");
            xstream.omitField(Ring.class, "m_callSequence");
            xstream.addImmutableType(Type.class);
            xstream.registerConverter(new RingTypeConverter());
        }

        @Override
        protected void configureImplicitCollections(XStream xstream) {
            xstream.addImplicitCollection(AbstractCallSequence.class, "m_rings", Ring.class);
        }
    }
}
