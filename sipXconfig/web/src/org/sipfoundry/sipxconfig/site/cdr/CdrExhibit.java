/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

import static org.sipfoundry.sipxconfig.components.TapestryUtils.getResponseOutputStream;

public abstract class CdrExhibit extends SipxBasePage {
    public static final Log LOG = LogFactory.getLog(CdrExhibit.class);

    @Asset(value = "context:/WEB-INF/cdr/Exhibit.script")
    public abstract IAsset getExhibitScript();

    @InjectObject(value = "spring:cdrManager")
    public abstract CdrManager getCdrManager();

    @Bean
    public abstract ExhibitDelegate getExhibitDelegate();

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    public void export() {
        try {
            OutputStream stream = getResponseOutputStream(getResponse(), "cdrs.js", "application/json");
            Writer out = new OutputStreamWriter(stream, "UTF-8");
            getCdrManager().dumpCdrsJson(out);
            out.close();
        } catch (IOException e) {
            LOG.error("Cannot export CDRs", e);
        }
    }

    public static class ExhibitDelegate implements IRender {
        private static final String[] SCRIPT_URLS = {
            "http://static.simile.mit.edu/exhibit/api-2.0/exhibit-api.js",
            "http://static.simile.mit.edu/exhibit/extensions-2.0/time/time-extension.js"
        };

        public void render(IMarkupWriter writer, IRequestCycle cycle) {
            for (String url : SCRIPT_URLS) {
                writer.begin("script");
                writer.appendAttribute("src", url);
                writer.appendAttribute("type", "text/javascript");
                writer.end();
            }
        }
    }
}
