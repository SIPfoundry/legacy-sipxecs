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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;

import com.thoughtworks.xstream.XStream;

import org.restlet.data.Form;
import org.restlet.data.MediaType;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PagedPhonebook;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;

public class UserPagedPhonebookResource extends UserPhonebookSearchResource {
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Form form = getRequest().getResourceRef().getQueryAsForm();
        String start = form.getFirstValue("start");
        String end = form.getFirstValue("end");
        String queryString = form.getFirstValue("filter");
        Collection<Phonebook> phonebooks = getPhonebookManager().getPublicPhonebooksByUser(getUser());
        PagedPhonebook pagedPhonebook = getPhonebookManager().getPagedPhonebook(phonebooks, getUser(), start, end,
                queryString);
        PrivatePagedPhonebook privatePagedPhonebook = new PrivatePagedPhonebook(pagedPhonebook.getSize(),
                pagedPhonebook.getFilteredSize(), pagedPhonebook.getStartRow(), pagedPhonebook.getEndRow(),
                convertPhonebookEntries(pagedPhonebook.getEntries()));
        return new PagedPhonebookRepresentation(variant.getMediaType(), privatePagedPhonebook);
    }

    static class PrivatePagedPhonebook implements Serializable {
        @SuppressWarnings("unused")
        private final int m_size;
        @SuppressWarnings("unused")
        private final int m_filteredSize;
        @SuppressWarnings("unused")
        private final int m_startRow;
        @SuppressWarnings("unused")
        private final int m_endRow;
        @SuppressWarnings("unused")
        private final ArrayList<Representable> m_entries;

        public PrivatePagedPhonebook(int size, int filteredSize, int startRow, int endRow,
                ArrayList<Representable> entries) {
            m_size = size;
            m_filteredSize = filteredSize;
            m_startRow = startRow;
            m_endRow = endRow;
            m_entries = entries;
        }
    }

    static class PagedPhonebookRepresentation extends XStreamRepresentation<PrivatePagedPhonebook> {
        public PagedPhonebookRepresentation(MediaType mediaType, PrivatePagedPhonebook object) {
            super(mediaType, object);
        }

        public PagedPhonebookRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, "m_id");
            xstream.alias("phonebook", PrivatePagedPhonebook.class);
            xstream.aliasField("filtered-size", PrivatePagedPhonebook.class, "filteredSize");
            xstream.aliasField("start-row", PrivatePagedPhonebook.class, "startRow");
            xstream.aliasField("end-row", PrivatePagedPhonebook.class, "endRow");
            xstream.alias("entry", Representable.class);
            xstream.aliasField("first-name", Representable.class, "firstName");
            xstream.aliasField("last-name", Representable.class, "lastName");
            xstream.aliasField("contact-information", Representable.class, "addressBookEntry");
            xstream.omitField(AddressBookEntry.class, "m_useBranchAddress");
        }
    }
}
