package org.sipfoundry;

import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;

/**
 * REST API that will provide a programming interface to your feature as well as
 * support to dart calls in UI web page.
 */
public class ExampleApi extends Resource {
    private Example m_example;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
    }
    
    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        return new StringRepresentation(m_example.hello());
    }
    
    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        try {
            IOUtils.toString(entity.getReader());
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }    

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    public void setExample(Example example) {
        m_example = example;
    };
}
