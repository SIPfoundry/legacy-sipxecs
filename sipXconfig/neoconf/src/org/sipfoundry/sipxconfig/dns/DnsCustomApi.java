/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.dns;

import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.IOException;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Collection;
import java.util.Collections;

import org.codehaus.jackson.map.ObjectMapper;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class DnsCustomApi extends Resource {
    private Integer m_customId;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private DnsManager m_dnsManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String id = (String) getRequest().getAttributes().get("id");
        if (id != null) {
            m_customId = Integer.valueOf(id);
        }
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        StringWriter json = new StringWriter();
        try {
            if (m_customId != null) {
                DnsCustomRecords custom = m_dnsManager.getCustomRecordsById(m_customId);
                m_jsonMapper.writeValue(json, Collections.singletonMap("custom", custom));
            } else {
                Collection<DnsCustomRecords> custom = m_dnsManager.getCustomRecords();
                writeCustom(json, custom);
            }
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        try {
            DnsCustomRecords custom = readCustom(entity.getReader());
            custom.setUniqueId(BeanWithId.UNSAVED_ID);
            m_dnsManager.saveCustomRecords(custom);

            // caller needs the object id so it can follow-up calls
            getResponse().setEntity(new StringRepresentation(custom.getId().toString()));
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        try {
            DnsCustomRecords custom = readCustom(entity.getReader());
            m_dnsManager.saveCustomRecords(custom);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    // DELETE
    @Override
    public void removeRepresentations() throws ResourceException {
        DnsCustomRecords custom = m_dnsManager.getCustomRecordsById(m_customId);
        m_dnsManager.deleteCustomRecords(custom);
    }

    DnsCustomRecords readCustom(Reader in) throws ResourceException, IOException {
        return m_jsonMapper.readValue(in, DnsCustomRecords.class);
    }

    void writeCustom(Writer json, Collection<DnsCustomRecords> custom) throws IOException {
        m_jsonMapper.writeValue(json, custom);
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    };

    @Override
    public boolean allowDelete() {
        return true;
    };

    @Override
    public boolean allowPut() {
        return true;
    };

    ObjectMapper getJsonMapper() {
        return m_jsonMapper;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }
}
