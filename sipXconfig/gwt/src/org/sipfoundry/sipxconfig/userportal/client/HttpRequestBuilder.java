/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import java.util.Map;

import com.google.gwt.core.client.GWT;
import com.google.gwt.http.client.Request;
import com.google.gwt.http.client.RequestBuilder;
import com.google.gwt.http.client.RequestCallback;
import com.google.gwt.http.client.RequestException;
import com.google.gwt.http.client.Response;
import com.smartgwt.client.util.SC;

import org.sipfoundry.sipxconfig.userportal.locale.SearchConstants;

// TODO: GWT RequestBuilder current does not support PUT and DELETE.
// This class is a hack to add support to allow PUT and DELETE
// requests. Once GWT fixes this issue, this class should be removed.
public final class HttpRequestBuilder extends RequestBuilder {

    public static final String PUT = "PUT";
    public static final String DELETE = "DELETE";
    public static final String POST = "POST";

    private static SearchConstants s_searchConstants = GWT.create(SearchConstants.class);

    private HttpRequestBuilder(String httpMethod, String url) {
        super(httpMethod, url);
    }

    public static void doPut(String url) {
        doPut(url, null);
    }

    public static void doPut(String url, String successMessage) {
        HttpRequestBuilder builder = new HttpRequestBuilder(PUT, url);
        sendRequest(builder, null, successMessage, null);
    }

    public static void doPut(String url, String postData, String contentType, String variant) {
        doPut(url, postData, contentType, variant, null);
    }

    public static void doPut(String url, String postData, String contentType, String variant, String successMessage) {
        doPut(url, postData, contentType, variant, successMessage, null);
    }

    public static void doPut(String url, String postData, String contentType, String variant, String successMessage,
            Map<Integer, String> statuses) {
        HttpRequestBuilder builder = new HttpRequestBuilder(PUT, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, postData, successMessage, statuses);
    }

    public static void doDelete(String url) {
        doDelete(url, null);
    }

    public static void doDelete(String url, String successMessage) {
        HttpRequestBuilder builder = new HttpRequestBuilder(DELETE, url);
        sendRequest(builder, null, successMessage, null);
    }

    public static void doDelete(String url, String contentType, String variant) {
        doDelete(url, contentType, variant, null);
    }

    public static void doDelete(String url, String contentType, String variant, String successMessage) {
        HttpRequestBuilder builder = new HttpRequestBuilder(DELETE, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, null, successMessage, null);
    }

    public static void doPost(String url, String postData, String contentType, String variant) {
        doPost(url, postData, contentType, variant, null);
    }

    public static void doPost(String url, String postData, String contentType, String variant, String successMessage) {
        doPost(url, postData, contentType, variant, successMessage, null);
    }

    public static void doPost(String url, String postData, String contentType, String variant,
            String successMessage, Map<Integer, String> errorStatuses) {
        HttpRequestBuilder builder = new HttpRequestBuilder(POST, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, postData, successMessage, errorStatuses);
    }

    private static void sendRequest(HttpRequestBuilder builder, String postData, final String successMessage,
            final Map<Integer, String> errorStatuses) {
        try {
            builder.sendRequest(postData, new RequestCallback() {
                public void onResponseReceived(Request request, Response response) {
                    int httpStatusCode = response.getStatusCode();
                    if (httpStatusCode != Response.SC_ACCEPTED && httpStatusCode != Response.SC_OK
                            && httpStatusCode != Response.SC_NO_CONTENT) {
                        StringBuilder errorMsg = new StringBuilder();
                        if (errorStatuses != null && errorStatuses.containsKey(httpStatusCode)) {
                            errorMsg.append(errorStatuses.get(httpStatusCode));
                        } else {
                            errorMsg.append(s_searchConstants.requestFailed() + "\n");
                            errorMsg.append(String.valueOf(httpStatusCode) + " " + response.getStatusText());
                        }
                        SC.warn(errorMsg.toString());
                    } else {
                        if (successMessage != null) {
                            SC.say(successMessage);
                        }
                    }
                }

                public void onError(Request request, Throwable exception) {
                    SC.warn(s_searchConstants.requestFailed() + exception.getMessage());
                }
            });
        } catch (RequestException e) {
            SC.warn(s_searchConstants.requestFailed() + e.getMessage());
        }
    }
}
