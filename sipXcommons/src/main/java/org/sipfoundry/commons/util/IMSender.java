/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.util;

import java.net.URL;

import org.sipfoundry.commons.userdb.User;

public class IMSender {

    private static HttpResult sendIM(User user, String instantMsg, String sendIMUrlStr, String context) throws Exception {
        if (sendIMUrlStr == null) {
            return new HttpResult(false, "no http url specified", null);
        }

        URL sendIMUrl = new URL(sendIMUrlStr + "/" + user.getUserName() + context);

        RemoteRequest rr = new RemoteRequest(sendIMUrl, "text/plain", instantMsg);
        boolean success = rr.http();
        return new HttpResult(success, rr.getResponse(), rr.getException());
    }

    public static HttpResult sendVmEntryIM(User user, String instantMsg, String sendIMUrlStr) throws Exception {
        return sendIM(user, instantMsg, sendIMUrlStr, "/SendVMEntryIM") ;
    }

    public static HttpResult sendVmExitIM(User user, String instantMsg, String sendIMUrlStr) throws Exception {
        return sendIM(user, instantMsg, sendIMUrlStr, "/SendVMExitIM") ;
    }

    public static HttpResult sendConfEntryIM(User user, String instantMsg, String sendIMUrlStr) throws Exception {
        return sendIM(user, instantMsg, sendIMUrlStr, "/SendConfEntryIM") ;
    }

    public static HttpResult sendConfExitIM(User user, String instantMsg, String sendIMUrlStr) throws Exception {
        return sendIM(user, instantMsg, sendIMUrlStr, "/SendConfExitIM") ;
    }

    public static final class HttpResult {
        boolean m_success;
        String response;
        Exception m_exception;

        public HttpResult(boolean success, String response, Exception exception) {
            super();
            m_success = success;
            this.response = response;
            m_exception = exception;
        }
        public boolean isSuccess() {
            return m_success;
        }
        public String getResponse() {
            return response;
        }
        public Exception getException() {
            return m_exception;
        }

    }

}
