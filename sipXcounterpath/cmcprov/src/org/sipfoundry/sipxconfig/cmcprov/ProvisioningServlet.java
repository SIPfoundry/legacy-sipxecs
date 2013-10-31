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
import java.io.InputStream;
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
import org.springframework.context.ApplicationContext;
import org.springframework.security.authentication.ProviderManager;
import org.springframework.web.context.support.WebApplicationContextUtils;

public abstract class ProvisioningServlet extends HttpServlet {
    public static final String SIPXCONFIG_SERVLET_PATH = "/sipxconfig";
    public static final String CORE_CONTEXT_BEAN_NAME = "coreContext";
    public static final String PHONE_CONTEXT_BEAN_NAME = "phoneContext";
    public static final String UPLOAD_BEAN_NAME = "upload";
    public static final String AUTH_MANAGER = "authenticationManager";
    public static final String DATA_SECTION = "[DATA]";
    public static final String QUOTE_CHAR = "\"";
    public static final String USERNAME = "username";
    public static final String PASSWORD = "password";
    public static final String UPDATE_SERVLET = "/update";

    public static final String EQUAL_SIGN = "=";

    private static ProvisioningContextImpl s_context;

    private ApplicationContext m_webContext;

    // private static final Log LOG = LogFactory.getLog(ProvisioningServlet.class);

    @Override
    public void init() {
        if (s_context == null) {
            ServletContext ctx = getServletContext();
            ServletContext sipxconfigCtx = ctx.getContext(SIPXCONFIG_SERVLET_PATH);
            m_webContext = WebApplicationContextUtils
                    .getRequiredWebApplicationContext(sipxconfigCtx);
            CoreContext sipxCoreContext = ((CoreContext) (m_webContext
                    .getBean(CORE_CONTEXT_BEAN_NAME)));
            PhoneContext sipxPhoneContext = ((PhoneContext) (m_webContext
                    .getBean(PHONE_CONTEXT_BEAN_NAME)));
            Upload sipxUpload = ((Upload) (m_webContext.getBean(UPLOAD_BEAN_NAME)));
            ProviderManager authManager = (ProviderManager) m_webContext.getBean(AUTH_MANAGER);
            s_context = new ProvisioningContextImpl();
            s_context.setSipxCoreContext(sipxCoreContext);
            s_context.setSipxPhoneContext(sipxPhoneContext);
            s_context.setAuthManager(authManager);
            s_context.setSipxUpload(sipxUpload);
        }
    }

    public static ProvisioningContext getProvisioningContext() {
        return s_context;
    }

    protected ApplicationContext getWebContext() {
        return m_webContext;
    }

    protected static void buildSuccessResponse(PrintWriter out, Map<String, String> settings) {
        out.println(DATA_SECTION);
        out.println("Success=1");
        out.println(StringUtils.EMPTY);
        out.println("[SETTINGS]");
        for (Map.Entry<String, String> e : settings.entrySet()) {
            out.println(e.getKey() + EQUAL_SIGN + QUOTE_CHAR + e.getValue() + QUOTE_CHAR);
        }
    }

    protected static void buildFailureResponse(PrintWriter out, String errorMessage) {
        out.println(DATA_SECTION);
        out.println("Success=0");
        if (errorMessage != null) {
            out.println("Failure=" + QUOTE_CHAR + errorMessage + QUOTE_CHAR);
        }
    }

    public static void attachFile(File file, Writer out, String user, String password) throws IOException {
        Reader in = new FileReader(file);
        String match = String.format("SIPX_%s_IM_PWD", user);
        InputStream is = IOUtils.toInputStream(IOUtils.toString(in).replace(match, password));
        IOUtils.copy(is, out);
        IOUtils.closeQuietly(is);
        IOUtils.closeQuietly(in);
    }

    public static void uploadPhoneProfile(String profileFilename, Writer out, String user, String password) {
        String uploadDirectory = getProvisioningContext().getUploadDirectory();
        try {
            attachFile(new File(uploadDirectory, profileFilename), out, user, password);
        } catch (IOException e) {
            throw new FailureDataException("Error while uploading configuration file: "
                    + e.getMessage());
        }
    }
}
