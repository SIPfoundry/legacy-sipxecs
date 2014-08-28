/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.rest;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpMethodBase;
import org.apache.commons.httpclient.methods.DeleteMethod;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.PutMethod;
import org.apache.commons.httpclient.methods.StringRequestEntity;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.beans.factory.annotation.Required;

public class RestRedirectorResource extends UserResource {
    public static final String CALLCONTROLLER = "/callcontroller";
    public static final String CDR = "/cdr";
    public static final String MAILBOX = "/mailbox";
    public static final String MEDIA = "/media";
    private static final String GET = "GET";
    private static final String PUT = "PUT";
    private static final String POST = "POST";
    private static final String DELETE = "DELETE";

    private static final Log LOG = LogFactory.getLog(RestRedirectorResource.class);

    private HttpInvoker m_httpInvoker;
    private AddressManager m_addressManager;
    private MailboxManager m_mailboxManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_httpInvoker = m_httpInvoker == null ? new HttpInvokerImpl() : m_httpInvoker;
        getVariants().add(new Variant(MediaType.ALL));
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPut() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    @Override
    public boolean allowDelete() {
        return true;
    }

    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String callcontrollerRelativeUrl = StringUtils.substringAfter(url, CALLCONTROLLER);

        if (!StringUtils.isEmpty(callcontrollerRelativeUrl)) {
            m_httpInvoker.invokePost(m_addressManager.getSingleAddress(RestServer.HTTP_API).toString()
                    + CALLCONTROLLER + callcontrollerRelativeUrl);
        }
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);

        if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            invokeIvrFallback(PUT, MAILBOX + mailboxRelativeUrl, entity);
        }
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String cdrRelativeUrl = StringUtils.substringAfter(url, CDR);
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);
        String callcontrollerRelativeUrl = StringUtils.substringAfter(url, CALLCONTROLLER);
        String mediaRelativeUrl = StringUtils.substringAfter(url, MEDIA);
        byte[] result = null;
        if (!StringUtils.isEmpty(cdrRelativeUrl)) {
            result = m_httpInvoker.invokeGet(m_addressManager.getSingleAddress(RestServer.HTTP_API) + CDR
                    + cdrRelativeUrl);
        } else if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            result = invokeIvrFallback(GET, MAILBOX + mailboxRelativeUrl);
        } else if (!StringUtils.isEmpty(callcontrollerRelativeUrl)) {
            result = m_httpInvoker.invokeGet(m_addressManager.getSingleAddress(RestServer.HTTP_API) + CALLCONTROLLER
                    + callcontrollerRelativeUrl);
        } else if (!StringUtils.isEmpty(mediaRelativeUrl)) {
            result = invokeIvrFallback(GET, MEDIA + mediaRelativeUrl);
        } else {
            throw new ResourceException(Status.CLIENT_ERROR_NOT_FOUND, "No known resource was requested");
        }
        return new InputRepresentation(new ByteArrayInputStream(result), MediaType.ALL);
    }

    private byte[] invokeIvrFallback(String methodType, String relativeUri) throws ResourceException {
        return invokeIvrFallback(methodType, relativeUri, null);
    }

    private byte[] invokeIvrFallback(String methodType, String relativeUri, Representation entity)
        throws ResourceException {
        byte[] result = null;
        Address ivrGoodAddress = m_mailboxManager.getLastGoodIvrNode();
        if (ivrGoodAddress != null) {
            try {
                result = invokeMethod(ivrGoodAddress, methodType, relativeUri, entity);
                return result;
            } catch (ResourceException ex) {
                // do not throw exception as we want to iterate through all ivr nodes
                LOG.warn("Cannot call last good ivr address: " + ivrGoodAddress);
            }
        }
        List<Address> ivrAddresses = m_addressManager.getAddresses(Ivr.REST_API);
        for (Address address : ivrAddresses) {
            try {
                if (ivrGoodAddress != null && address.equals(ivrGoodAddress)) {
                    continue;
                }
                result = invokeMethod(address, methodType, relativeUri, entity);
                m_mailboxManager.setLastGoodIvrNode(address);
                return result;
            } catch (ResourceException ex) {
                LOG.warn("Cannot call ivr address: " + address);
            }
        }

        throw new ResourceException(Status.CONNECTOR_ERROR_COMMUNICATION, "No IVR node is running");
    }

    private byte[] invokeMethod(Address address, String methodType, String relativeUri, Representation entity)
        throws ResourceException {
        if (StringUtils.equals(methodType, GET)) {
            return m_httpInvoker.invokeGet(address.toString() + relativeUri);
        } else if (StringUtils.equals(methodType, POST)) {
            m_httpInvoker.invokePost(address.toString() + relativeUri);
            return null;
        } else if (StringUtils.equals(methodType, PUT)) {
            String payload = null;
            if (entity != null) {
                try {
                    payload = IOUtils.toString(entity.getReader());
                } catch (Exception ex) {
                    payload = null;
                }
            }
            m_httpInvoker.invokePut(address.toString() + relativeUri, payload);
            return null;
        } else if (StringUtils.equals(methodType, DELETE)) {
            m_httpInvoker.invokeDelete(address.toString() + relativeUri);
            return null;
        }
        return null;
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);

        if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            invokeIvrFallback(DELETE, MAILBOX + mailboxRelativeUrl);
        }
    }

    public interface HttpInvoker {
        public byte[] invokeGet(String address) throws ResourceException;

        public void invokePut(String address, String payload) throws ResourceException;

        public void invokePost(String address) throws ResourceException;

        public void invokeDelete(String address) throws ResourceException;
    }

    public class HttpInvokerImpl implements HttpInvoker {

        @Override
        public byte[] invokeGet(String address) throws ResourceException {
            HttpMethodBase method = new GetMethod(address.toString());
            return invokeRestService(method);
        }

        @Override
        public void invokePut(String address, String payload) throws ResourceException {
            PutMethod method = new PutMethod(address.toString());
            if (payload != null) {
                method.setRequestEntity(new StringRequestEntity(payload));
            }
            invokeRestService(method);
        }

        @Override
        public void invokePost(String address) throws ResourceException {
            HttpMethodBase method = new PostMethod(address.toString());
            invokeRestService(method);
        }

        @Override
        public void invokeDelete(String address) throws ResourceException {
            HttpMethodBase method = new DeleteMethod(address.toString());
            invokeRestService(method);
        }

        private byte[] invokeRestService(HttpMethodBase method) throws ResourceException {
            byte[] response = null;
            InputStream stream = null;
            ByteArrayOutputStream outputStream = null;
            try {
                HttpClient client = new HttpClient();
                method.setRequestHeader("sipx-user", getUser().getUserName());
                int status = client.executeMethod(method);
                stream = method.getResponseBodyAsStream();
                outputStream = new ByteArrayOutputStream();
                int n;
                byte[] buffer = new byte[1024];
                while ((n = stream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, n);
                }
                getResponse().setStatus(new Status(status));
                response = outputStream.toByteArray();
            } catch (IOException e) {
                throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST);
            } finally {
                if (method != null) {
                    method.releaseConnection();
                }
                IOUtils.closeQuietly(stream);
                IOUtils.closeQuietly(outputStream);
            }
            return response;
        }
    }

    public void setHttpInvoker(HttpInvoker httpInvoker) {
        m_httpInvoker = httpInvoker;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Required
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
