/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import com.thoughtworks.xstream.XStream;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

public class ContactInformationResource extends UserResource {

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        AddressBookEntry addressBook = getUser().getAddressBookEntry();
        AddressBookEntry reprAddressBook = (AddressBookEntry) addressBook.duplicate();
        return new AddressBookRepresentation(variant.getMediaType(), reprAddressBook);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        AddressBookRepresentation representation = new AddressBookRepresentation(entity);
        AddressBookEntry newAddressBook = representation.getObject();

        User user = getUser();
        AddressBookEntry addressBook = user.getAddressBookEntry();

        if (addressBook == null) {
            user.setAddressBookEntry(newAddressBook);
        } else {
            addressBook.update(newAddressBook);
            user.setAddressBookEntry(addressBook);
        }
        getCoreContext().saveUser(user);
    }

    static class AddressBookRepresentation extends XStreamRepresentation<AddressBookEntry> {
        public AddressBookRepresentation(MediaType mediaType, AddressBookEntry object) {
            super(mediaType, object);
        }

        public AddressBookRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, "m_id");
            xstream.alias("contact-information", AddressBookEntry.class);
            xstream.alias("homeAddress", Address.class);
            xstream.alias("officeAddress", Address.class);
        }
    }
}
