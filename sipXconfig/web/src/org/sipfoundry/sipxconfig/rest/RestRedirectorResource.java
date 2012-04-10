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

import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpMethodBase;
import org.apache.commons.httpclient.methods.DeleteMethod;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;
import org.apache.commons.httpclient.methods.PutMethod;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;

public class RestRedirectorResource extends UserResource {
    private static final String CALLCONTROLLER = "callcontroller";
    private static final String CDR = "cdr";
    private static final String MAILBOX = "mailbox";

    private static final String TRUSTED_HOST = "localhost";

    private HttpInvoker m_httpInvoker;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        m_httpInvoker = m_httpInvoker == null ? new HttpInvokerImpl() : m_httpInvoker;
        getVariants().add(new Variant(MediaType.TEXT_ALL));
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
            m_httpInvoker.invokePost(6666, CALLCONTROLLER + callcontrollerRelativeUrl);
        }
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);

        if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            m_httpInvoker.invokePut(8085, MAILBOX + mailboxRelativeUrl);
        }
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String cdrRelativeUrl = StringUtils.substringAfter(url, CDR);
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);
        String callcontrollerRelativeUrl = StringUtils.substringAfter(url, CALLCONTROLLER);
        String result = null;
        if (!StringUtils.isEmpty(cdrRelativeUrl)) {
            result = m_httpInvoker.invokeGet(6666, CDR + cdrRelativeUrl);
        } else if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            result = m_httpInvoker.invokeGet(8085, MAILBOX + mailboxRelativeUrl);
        } else if (!StringUtils.isEmpty(callcontrollerRelativeUrl)) {
            result = m_httpInvoker.invokeGet(6666, CALLCONTROLLER + callcontrollerRelativeUrl);
        }
        return new StringRepresentation(result);
    }

    @Override
    public void removeRepresentations() throws ResourceException {
        String url = getRequest().getResourceRef().getIdentifier();
        String mailboxRelativeUrl = StringUtils.substringAfter(url, MAILBOX);

        if (!StringUtils.isEmpty(mailboxRelativeUrl)) {
            m_httpInvoker.invokeDelete(8085, MAILBOX + mailboxRelativeUrl);
        }
    }

    public interface HttpInvoker {
        public String invokeGet(int port, String uri) throws ResourceException;
        public void invokePut(int port, String uri) throws ResourceException;
        public void invokePost(int port, String uri) throws ResourceException;
        public void invokeDelete(int port, String uri) throws ResourceException;
    }

    public class HttpInvokerImpl implements HttpInvoker {

        private String getRestServerUrl(String fqdn, int port, String uri) {
            return String.format("https://%s:%d/" + uri, fqdn, port);
        }

        @Override
        public String invokeGet(int port, String uri) throws ResourceException {
            HttpMethodBase method = new GetMethod(getRestServerUrl(TRUSTED_HOST, port, uri));
            return invokeRestService(method);
        }

        @Override
        public void invokePut(int port, String uri) throws ResourceException {
            HttpMethodBase method = new PutMethod(getRestServerUrl(TRUSTED_HOST, port, uri));
            invokeRestService(method);
        }

        @Override
        public void invokePost(int port, String uri) throws ResourceException {
            HttpMethodBase method = new PostMethod(getRestServerUrl(TRUSTED_HOST, port, uri));
            invokeRestService(method);
        }

        @Override
        public void invokeDelete(int port, String uri) throws ResourceException {
            HttpMethodBase method = new DeleteMethod(getRestServerUrl(TRUSTED_HOST, port, uri));
            invokeRestService(method);
        }

        private String invokeRestService(HttpMethodBase method) throws ResourceException {
            String response = null;
            InputStream stream = null;
            try {
                HttpClient client = new HttpClient();
                method.setRequestHeader("sipx-user", getUser().getUserName());
                int status = client.executeMethod(method);
                stream = method.getResponseBodyAsStream();
                response = IOUtils.toString(stream);
                getResponse().setStatus(new Status(status));
                getResponse().setEntity(response, MediaType.TEXT_PLAIN);
            } catch (IOException e) {
                throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST);
            } finally {
                if (method != null) {
                    method.releaseConnection();
                }
                IOUtils.closeQuietly(stream);
            }
            return response;
        }
    }

    public void setHttpInvoker(HttpInvoker httpInvoker) {
        m_httpInvoker = httpInvoker;
    }
}
