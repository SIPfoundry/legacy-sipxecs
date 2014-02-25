package org.sipfoundry.sipxconfig.rest;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;

public class PhonebookEntryResourceTest extends TestCase {

    public void testStoreRepresentation() throws ResourceException {
        AddressBookEntry abe = new AddressBookEntry();
        PhonebookEntry pbe = new PhonebookEntry();
        pbe.setAddressBookEntry(abe);
        Phonebook pb = new Phonebook();
        pbe.setPhonebook(pb);

        PhonebookManager manager = EasyMock.createMock(PhonebookManager.class);
        manager.getPhonebookEntry(1);
        EasyMock.expectLastCall().andReturn(pbe);
        manager.getPrivatePhonebookCreateIfRequired("200");
        EasyMock.expectLastCall().andReturn(pb);
        manager.savePhonebookEntry(pbe);
        EasyMock.expectLastCall();

        EasyMock.replay(manager);
        final InputStream xmlStream = getClass().getResourceAsStream("vcard.put.rest.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);

        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("Content-type", "text/xml; charset=utf-8");
        attributes.put("Accept", "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2");
        request.setAttributes(attributes);

        PhonebookEntryResource resource = new PhonebookEntryResource();
        resource.setPhonebookManager(manager);
        resource.init(null, request , new Response(request));
        resource.storeRepresentation(entity);

        EasyMock.verify(manager);

    }

    public void testStoreRepresentationNew() throws ResourceException {
        AddressBookEntry abe = new AddressBookEntry();
        PhonebookEntry pbe = new PhonebookEntry();
        pbe.setAddressBookEntry(abe);
        Phonebook pb = new Phonebook();
        pbe.setPhonebook(pb);

        PhonebookManager manager = EasyMock.createMock(PhonebookManager.class);
        manager.findPhonebookEntryByInternalId("XXX");
        EasyMock.expectLastCall().andReturn(pbe);
        manager.getPrivatePhonebookCreateIfRequired("200");
        EasyMock.expectLastCall().andReturn(pb);
        manager.savePhonebookEntry(pbe);
        EasyMock.expectLastCall();

        EasyMock.replay(manager);
        final InputStream xmlStream = getClass().getResourceAsStream("vcard.new.put.rest.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);

        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("Content-type", "text/xml; charset=utf-8");
        attributes.put("Accept", "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2");
        request.setAttributes(attributes);

        PhonebookEntryResource resource = new PhonebookEntryResource();
        resource.setPhonebookManager(manager);
        resource.init(null, request , new Response(request));
        resource.storeRepresentation(entity);

        EasyMock.verify(manager);

    }

    public void testStoreRepresentationNewNullPbe() throws ResourceException {
        AddressBookEntry abe = new AddressBookEntry();
        PhonebookEntry pbe = new PhonebookEntry();
        pbe.setAddressBookEntry(abe);
        Phonebook pb = new Phonebook();
        pbe.setPhonebook(pb);

        PhonebookManager manager = EasyMock.createMock(PhonebookManager.class);
        manager.findPhonebookEntryByInternalId("XXX");
        EasyMock.expectLastCall().andReturn(pbe);
        manager.getPrivatePhonebookCreateIfRequired("200");
        EasyMock.expectLastCall().andReturn(null);
        manager.savePhonebookEntry(pbe);
        EasyMock.expectLastCall();

        EasyMock.replay(manager);
        final InputStream xmlStream = getClass().getResourceAsStream("vcard.new.put.rest.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);

        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("Content-type", "text/xml; charset=utf-8");
        attributes.put("Accept", "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2");
        request.setAttributes(attributes);

        PhonebookEntryResource resource = new PhonebookEntryResource();
        resource.setPhonebookManager(manager);
        resource.init(null, request , new Response(request));
        resource.storeRepresentation(entity);

        EasyMock.verify(manager);

    }

    public void testRemoveRepresentation() throws ResourceException {
        AddressBookEntry abe = new AddressBookEntry();
        PhonebookEntry pbe = new PhonebookEntry();
        pbe.setAddressBookEntry(abe);
        Phonebook pb = new Phonebook();
        pbe.setPhonebook(pb);

        PhonebookManager manager = EasyMock.createMock(PhonebookManager.class);
        manager.findPhonebookEntryByInternalId("1");
        EasyMock.expectLastCall().andReturn(pbe);

        manager.deletePhonebookEntry(pbe);
        EasyMock.expectLastCall();
        EasyMock.replay(manager);
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("Content-type", "text/xml; charset=utf-8");
        attributes.put("Accept", "text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2");
        attributes.put("internalId", "1");
        request.setAttributes(attributes);

        PhonebookEntryResource resource = new PhonebookEntryResource();
        resource.setPhonebookManager(manager);
        resource.init(null, request , new Response(request));
        resource.removeRepresentations();

        EasyMock.verify(manager);
    }
}
