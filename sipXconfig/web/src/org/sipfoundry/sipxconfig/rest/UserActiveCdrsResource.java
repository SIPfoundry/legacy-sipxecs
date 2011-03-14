/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.thoughtworks.xstream.XStream;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.springframework.beans.factory.annotation.Required;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class UserActiveCdrsResource extends UserResource {

    private static final String RECIPIENT = "recipient";
    private static final String DURATION = "duration";
    private CdrManager m_cdrManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        List<Representable> representableList = null;
        try {
            List<Cdr> cdrs = m_cdrManager.getActiveCallsREST(getUser());
            representableList = convertCdrs(cdrs);
        } catch (IOException ex) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, ex);
        }
        return new CdrRepresentation(variant.getMediaType(), representableList);
    }

    protected ArrayList<Representable> convertCdrs(List<Cdr> cdrs) {
        ArrayList<Representable> cdrsArray = new ArrayList<Representable>();
        for (Cdr cdr : cdrs) {
            cdrsArray.add(new Representable(cdr));
        }
        return cdrsArray;
    }

    static class Representable implements Serializable {
        private String m_callee;
        private String m_calleeAor;
        private String m_calleeRoute;
        private String m_callDirection;
        private String m_recipient;
        private String m_caller;
        private String m_callerAor;
        private boolean m_callerInternal;
        private String m_callTypeName;
        private long m_startTime;
        private long m_duration;

        public Representable(Cdr cdr) {
            m_callee = cdr.getCallee();
            m_calleeAor = cdr.getCalleeAor();
            m_calleeRoute = cdr.getCalleeRoute();
            m_callDirection = cdr.getCallDirection();
            m_recipient = cdr.getRecipient();
            m_caller = cdr.getCaller();
            m_callerAor = cdr.getCallerAor();
            m_callerInternal = cdr.getCallerInternal();
            m_callTypeName = cdr.getCallTypeName();
            //put timestamp long value here, the REST client will parse it in any format wanted for display
            m_startTime = cdr.getStartTime().getTime();
            m_duration = cdr.getDuration();
        }
    }

    static class CdrRepresentation extends XStreamRepresentation<Collection<Representable>> {

        public CdrRepresentation(MediaType mediaType, Collection<Representable> object) {
            super(mediaType, object);
        }

        public CdrRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias("cdrs", List.class);
            xstream.alias("cdr", Representable.class);
            xstream.aliasField("from", Representable.class, "callee");
            xstream.aliasField("from-aor", Representable.class, "calleeAor");
            xstream.aliasField("to", Representable.class, "caller");
            xstream.aliasField("to-aor", Representable.class, "callerAor");
            xstream.aliasField("route", Representable.class, "calleeRoute");
            xstream.aliasField("direction", Representable.class, "callDirection");
            xstream.aliasField(RECIPIENT, Representable.class, RECIPIENT);
            xstream.aliasField("internal", Representable.class, "callerInternal");
            xstream.aliasField("type", Representable.class, "callTypeName");
            xstream.aliasField("start-time", Representable.class, "startTime");
            xstream.aliasField(DURATION, Representable.class, DURATION);
        }
    }

    @Required
    public void setCdrManager(CdrManager cdrManager) {
        m_cdrManager = cdrManager;
    }
}
