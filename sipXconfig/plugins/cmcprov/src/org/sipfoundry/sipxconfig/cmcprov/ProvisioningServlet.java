/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.Reader;
import java.io.Writer;
import java.util.Map;

import javax.servlet.ServletContext;
import javax.servlet.http.HttpServlet;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

public abstract class ProvisioningServlet extends HttpServlet {
    public static final String SIPXCONFIG_SERVLET_PATH = "/sipxconfig";
    public static final String CORE_CONTEXT_BEAN_NAME = "coreContext";
    public static final String PHONE_CONTEXT_BEAN_NAME = "phoneContext";
    public static final String UPLOAD_BEAN_NAME = "upload";
    public static final String DATA_SECTION = "[DATA]";
    public static final String QUOTE_CHAR = "\"";
    public static final String USERNAME = "username";
    public static final String PASSWORD = "password";
    public static final String UPDATE_SERVLET = "/update";

    public static final String EQUAL_SIGN = "=";

    private static ProvisioningContextImpl s_context;

    // private static final Log LOG = LogFactory.getLog(ProvisioningServlet.class);

    @Override
    public void init() {
        if (s_context == null) {
            ServletContext ctx = getServletContext();
            ServletContext sipxconfigCtx = ctx.getContext(SIPXCONFIG_SERVLET_PATH);
            WebApplicationContext webContext = WebApplicationContextUtils
                    .getRequiredWebApplicationContext(sipxconfigCtx);
            CoreContext sipxCoreContext = ((CoreContext) (webContext
                    .getBean(CORE_CONTEXT_BEAN_NAME)));
            PhoneContext sipxPhoneContext = ((PhoneContext) (webContext
                    .getBean(PHONE_CONTEXT_BEAN_NAME)));
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
        out.println(DATA_SECTION);
        out.println("Success=1");
        out.println(StringUtils.EMPTY);
        out.println("[SETTINGS]");
        for (Map.Entry<String, String> e : settings.entrySet()) {
            out.println(e.getKey() + EQUAL_SIGN + QUOTE_CHAR + e.getValue() + QUOTE_CHAR);
        }
    }

    protected void buildFailureResponse(PrintWriter out, String errorMessage) {
        out.println(DATA_SECTION);
        out.println("Success=0");
        if (errorMessage != null) {
            out.println("Failure=" + QUOTE_CHAR + errorMessage + QUOTE_CHAR);
        }
    }

    public void attachFile(File file, Writer out) throws IOException {
        Reader in = new FileReader(file);
        IOUtils.copy(in, out);
        IOUtils.closeQuietly(in);
    }

    public void uploadPhoneProfile(String profileFilename, Writer out) {
        String uploadDirectory = getProvisioningContext().getUploadDirectory();
        try {
            attachFile(new File(uploadDirectory, profileFilename), out);
        } catch (IOException e) {
            throw new FailureDataException("Error while uploading configuration file: "
                    + e.getMessage());
        }
    }
}
