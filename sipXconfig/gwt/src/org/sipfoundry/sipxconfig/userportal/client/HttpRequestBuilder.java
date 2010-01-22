/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import com.google.gwt.core.client.GWT;
import com.google.gwt.http.client.Request;
import com.google.gwt.http.client.RequestBuilder;
import com.google.gwt.http.client.RequestCallback;
import com.google.gwt.http.client.RequestException;
import com.google.gwt.http.client.Response;
import com.google.gwt.user.client.Window;

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
        HttpRequestBuilder builder = new HttpRequestBuilder(PUT, url);
        sendRequest(builder, null);
    }

    public static void doPut(String url, String postData, String contentType, String variant) {
        HttpRequestBuilder builder = new HttpRequestBuilder(PUT, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, postData);
    }

    public static void doDelete(String url) {
        HttpRequestBuilder builder = new HttpRequestBuilder(DELETE, url);
        sendRequest(builder, null);
    }

    public static void doDelete(String url, String contentType, String variant) {
        HttpRequestBuilder builder = new HttpRequestBuilder(DELETE, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, null);
    }

    public static void doPost(String url, String postData, String contentType, String variant) {
        HttpRequestBuilder builder = new HttpRequestBuilder(POST, url);
        builder.setHeader(contentType, variant);
        sendRequest(builder, postData);
    }

    private static void sendRequest(HttpRequestBuilder builder, String postData) {
        try {
            builder.sendRequest(postData, new RequestCallback() {
                public void onResponseReceived(Request request, Response response) {
                    int httpStatusCode = response.getStatusCode();
                    if (httpStatusCode != Response.SC_ACCEPTED && httpStatusCode != Response.SC_OK
                            && httpStatusCode != Response.SC_NO_CONTENT) {
                        StringBuilder errorMsg = new StringBuilder(s_searchConstants.requestFailed());
                        errorMsg.append("\n" + String.valueOf(httpStatusCode) + " " + response.getStatusText());
                        Window.alert(errorMsg.toString());
                    }
                }

                public void onError(Request request, Throwable exception) {
                    Window.alert(s_searchConstants.requestFailed() + exception.getMessage());
                }
            });
        } catch (RequestException e) {
            Window.alert(s_searchConstants.requestFailed() + e.getMessage());
        }
    }
}
