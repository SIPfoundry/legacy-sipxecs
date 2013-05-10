/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;

public class OpenAcdConfigurationTest {

    private static final String NEW_LINE = "\n";

    @Test
    public void testConfigWithNoPlugins() throws Exception {
        OpenAcdConfiguration config = new OpenAcdConfiguration();
        StringWriter actual = new StringWriter();
        config.writeConfigFile(actual, new ArrayList<String>(), new ArrayList<String>());
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sys-config"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testConfigWithPlugin() throws Exception {
        OpenAcdConfiguration config = new OpenAcdConfiguration();
        StringWriter actual = new StringWriter();
        List<String> pluginDefinitions = new ArrayList<String>();
        List<String> pluginConfigurations = new ArrayList<String>();
        pluginDefinitions.add("oacd_ouc");
        pluginConfigurations.add(getOpenAcdPluginConfiguration());
        config.writeConfigFile(actual, pluginDefinitions, pluginConfigurations);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-sys-config-with-plugins"));
        assertEquals(expected, actual.toString());
    }

    private String getOpenAcdPluginConfiguration() {
        StringBuilder builder = new StringBuilder();
        builder.append("{oacd_ouc, [");
        builder.append(NEW_LINE);
        builder.append("    {sipxconfig_rest_api, \"http://$(sipx.master_fqdn)/sipxconfig/rest\"},");
        builder.append(NEW_LINE);
        builder.append("    {contact_info_resource, \"/my/contact-information\"},");
        builder.append(NEW_LINE);
        builder.append("    {root_uri, \"/openacd\"},");
        builder.append(NEW_LINE);
        builder.append("");
        builder.append(NEW_LINE);
        builder.append("    % enable js debug tools/logs");
        builder.append(NEW_LINE);
        builder.append("    % {frontend_debug, true},");
        builder.append(NEW_LINE);
        builder.append("");
        builder.append(NEW_LINE);
        builder.append("    % static file server (no trailing slash)");
        builder.append(NEW_LINE);
        builder.append("    {frontend_static_root_uri, \"http://$(sys.fqhost)/openacd/static\"}");
        builder.append(NEW_LINE);
        builder.append("]}");

        return builder.toString();
    }
}
