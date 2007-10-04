/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.Collection;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public class PhonebookResource extends Resource {
    private static final MediaType CSV = new MediaType("text/comma-separated-values");
    private static final String[] HEADERS = new String[] {
        "First name",
        "Last name",
        "Number"
    };
    private PhonebookManager m_phonebookManager;
    private Phonebook m_phonebook;
    
    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        String name = (String) getRequest().getAttributes().get("name");
        m_phonebook = m_phonebookManager.getPhonebookByName(name);
        getVariants().add(new Variant(CSV));
    }
    
    @Override
    public Representation getRepresentation(Variant variant) {
        return new PhonebookCsv(m_phonebookManager.getEntries(m_phonebook));
    }
    
    static final class PhonebookCsv extends OutputRepresentation {
        private Collection<PhonebookEntry> m_entries;
        PhonebookCsv(Collection<PhonebookEntry> entries) {
            super(CSV);
            m_entries = entries;
        }

        @Override
        public void write(OutputStream outputStream) throws IOException {
            Writer writer = new OutputStreamWriter(outputStream);
            CsvWriter csv = new CsvWriter(writer);
            csv.write(HEADERS, true);            
            for (PhonebookEntry entry : m_entries) {
                try {
                    String[] fields = new String[] {
                            entry.getFirstName(),
                            entry.getLastName(),
                            entry.getNumber()
                    };
                    csv.write(fields, true);
                } catch (RuntimeException e) {
                    throw e;
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
            }
            writer.flush();
        }        
    }

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }
}
