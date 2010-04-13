/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rest;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.text.DateFormat;
import java.util.Date;
import java.util.Locale;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import static org.restlet.data.MediaType.TEXT_PLAIN;

public class ConfigServerTimeResource extends UserResource {

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_PLAIN));
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        TimeTextRepresentation timeRest = new TimeTextRepresentation();
        return timeRest;
    }

    static final class TimeTextRepresentation extends OutputRepresentation {

        TimeTextRepresentation() {
            super(TEXT_PLAIN);
        }

        @Override
        public void write(OutputStream outputStream) throws IOException {
            Writer writer = new OutputStreamWriter(outputStream);
            DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.DEFAULT,
                    DateFormat.SHORT, Locale.getDefault());
          //TODO This is formating the datetime with server locale need to send page locale in URL
            writer.write(dateFormat.format(new Date()));
            writer.flush();
        }
    }

}
