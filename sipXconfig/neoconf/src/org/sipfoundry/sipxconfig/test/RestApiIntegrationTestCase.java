/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.test;

import java.util.Arrays;

import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

public abstract class RestApiIntegrationTestCase extends IntegrationTestCase {
    private static final String URI = "http://localhost:9000/api/%s";
    private RestTemplate m_template;

    protected int postJsonString(String json, String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.POST,
                createRequest(json, MediaType.APPLICATION_JSON), Integer.class);
        return response.getStatusCode().value();
    }

    protected int postXmlString(String xml, String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.POST,
                createRequest(xml, MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML), Integer.class);
        return response.getStatusCode().value();
    }

    protected int putJsonString(String json, String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.PUT,
                createRequest(json, MediaType.APPLICATION_JSON), Integer.class);
        return response.getStatusCode().value();
    }

    protected int putXmlString(String xml, String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.PUT,
                createRequest(xml, MediaType.APPLICATION_JSON, MediaType.APPLICATION_XML), Integer.class);
        return response.getStatusCode().value();
    }

    protected int putPlainText(String text, String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.PUT,
                createRequest(text, MediaType.APPLICATION_JSON, MediaType.TEXT_PLAIN), Integer.class);
        return response.getStatusCode().value();
    }

    protected int delete(String url) throws RestClientException {
        ResponseEntity<Integer> response = m_template.exchange(String.format(URI, url), HttpMethod.DELETE, null,
                Integer.class);
        return response.getStatusCode().value();
    }

    protected String getAsJson(String url) {
        ResponseEntity<String> response = m_template.exchange(String.format(URI, url), HttpMethod.GET,
                createEntity(MediaType.APPLICATION_JSON), String.class);
        return response.getBody();
    }

    protected String getAsXml(String url) {
        ResponseEntity<String> response = m_template.exchange(String.format(URI, url), HttpMethod.GET,
                createEntity(MediaType.APPLICATION_XML), String.class);
        return response.getBody();
    }

    @Override
    protected String[] getConfigLocations() {
        // load jaxrs test server
        return new String[] {
            "classpath*:**/*.beans.xml", "classpath:/org/sipfoundry/sipxconfig/api/jaxrs-server-test.xml"
        };
    }

    private HttpEntity<String> createEntity(MediaType mediaType) {
        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(mediaType);
        headers.setAccept(Arrays.asList(mediaType));
        return new HttpEntity<String>("parameters", headers);
    }

    private HttpEntity<String> createRequest(String content, MediaType mediaType) {
        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(mediaType);
        headers.setAccept(Arrays.asList(mediaType));
        return new HttpEntity<String>(content, headers);
    }

    private HttpEntity<String> createRequest(String content, MediaType accept, MediaType mediaType) {
        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(mediaType);
        headers.setAccept(Arrays.asList(accept));
        return new HttpEntity<String>(content, headers);
    }

    public void setRestTemplate(RestTemplate template) {
        m_template = template;
    }
}
