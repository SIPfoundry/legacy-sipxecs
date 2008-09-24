/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.PrintWriter;
import java.util.Map;

import javax.servlet.ServletContext;
import javax.servlet.http.HttpServlet;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

public abstract class ProvisioningServlet extends HttpServlet {
    public static final String SIPXCONFIG_SERVLET_PATH = "/sipxconfig";
    public static final String CORE_CONTEXT_BEAN_NAME = "coreContextImpl";
    public static final String PHONE_CONTEXT_BEAN_NAME = "phoneContextImpl";
    public static final String UPLOAD_BEAN_NAME = "upload";
    public static final String DATA_SECTION = "[DATA]";
    public static final String QUOTE_CHAR = "\"";

    private static ProvisioningContextImpl s_context;

    private static final Log LOG = LogFactory.getLog(ProvisioningServlet.class);

    public void init() {
        if (s_context == null) {
            ServletContext ctx = getServletContext();
            ServletContext sipxconfigCtx = ctx.getContext(SIPXCONFIG_SERVLET_PATH);
            WebApplicationContext webContext = WebApplicationContextUtils
                .getRequiredWebApplicationContext(sipxconfigCtx);
            CoreContext sipxCoreContext = ((CoreContext) (webContext.getBean(CORE_CONTEXT_BEAN_NAME)));
            PhoneContext sipxPhoneContext = ((PhoneContext) (webContext.getBean(PHONE_CONTEXT_BEAN_NAME)));
            Upload sipxUpload = ((Upload) (webContext.getBean(UPLOAD_BEAN_NAME)));
            s_context = new ProvisioningContextImpl();
            s_context.setSipxCoreContext(sipxCoreContext);
            s_context.setSipxPhoneContext(sipxPhoneContext);
            s_context.setSipxUpload(sipxUpload);
        }
    }

    public ProvisioningContext getProvisioningContext() {
        return s_context;
    }

    protected void buildSuccessResponse(PrintWriter out, Map<String, String> settings) {
        StringBuilder sb = new StringBuilder();
        sb.append(DATA_SECTION + "\n");
        sb.append("Success=1\n");
        sb.append("\n");
        sb.append("[SETTINGS]\n");
        out.println(DATA_SECTION);
        out.println("Success=1");
        out.println("");
        out.println("[SETTINGS]");
        for (Map.Entry<String, String> e : settings.entrySet()) {
            out.println(e.getKey() + "=" + QUOTE_CHAR + e.getValue() + QUOTE_CHAR);
            sb.append("" + e.getKey() + "=" + QUOTE_CHAR + e.getValue() + QUOTE_CHAR + "\n");
        }
        LOG.info("RASPUNS::::\n" + sb.toString() + "\n");
    }

    protected void buildFailureResponse(PrintWriter out, String errorMessage) {
        out.println(DATA_SECTION);
        out.println("Success=0");
        if (errorMessage != null) {
            out.println("Failure=" + QUOTE_CHAR + errorMessage + QUOTE_CHAR);
        }
    }
}
