/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
import java.util.Collection;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.OutputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

import static org.restlet.data.MediaType.*;

public class PhonebookResource extends Resource {
    private static final MediaType CSV = new MediaType("text/comma-separated-values");
    private static final String[] HEADERS = new String[] {
        "First name", "Last name", "Number"
    };
    private PhonebookManager m_phonebookManager;
    private Phonebook m_phonebook;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        String name = (String) getRequest().getAttributes().get("name");
        m_phonebook = m_phonebookManager.getPhonebookByName(name);
        if (m_phonebook == null) {
            setAvailable(false);
        }
        getVariants().add(new Variant(CSV));
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_ALL_XML));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        final Collection<PhonebookEntry> entries = m_phonebookManager.getEntries(m_phonebook);
        MediaType mediaType = variant.getMediaType();
        if (CSV.equals(mediaType)) {
            return new PhonebookCsv(entries);
        }
        return new Dom4jRepresentation(getPhonebookXml(entries));
    }

    static Document getPhonebookXml(Collection<PhonebookEntry> entries) {
        DocumentFactory factory = DocumentFactory.getInstance();
        Document document = factory.createDocument();
        Element rootItem = document.addElement("phonebook");
        for (PhonebookEntry entry : entries) {
            Element entryEl = rootItem.addElement("entry");
            entryEl.addElement("first-name").setText(entry.getFirstName());
            entryEl.addElement("last-name").setText(entry.getLastName());
            entryEl.addElement("number").setText(entry.getNumber());
        }

        return document;
    }

    static final class PhonebookCsv extends OutputRepresentation {
        private final Collection<PhonebookEntry> m_entries;

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
                        entry.getFirstName(), entry.getLastName(), entry.getNumber()
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
