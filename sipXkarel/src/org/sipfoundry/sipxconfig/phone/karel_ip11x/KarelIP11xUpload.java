/*
 *
 *
 * Copyright (C) 2011 Karel Electronics Corp., All rights reserved.
 *
 **/
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.upload.Upload;

public class KarelIP11xUpload extends Upload {

    private static final String FIRMWARE_ROM_SETTING = "firmware/rom";

    private static final Log LOG = LogFactory.getLog(KarelIP11xUpload.class);

    private String m_profileLocation;

    private KarelIP11xModelConfiguration m_configuration;

    public void setConfiguration(KarelIP11xModelConfiguration configuration) {
        m_configuration = configuration;
    }

    public KarelIP11xModelConfiguration getConfiguration() {
        return m_configuration;
    }

    public void setProfileLocation(String profileLocation) {
        m_profileLocation = profileLocation;
    }

    public String getProfileLocation() {
        return m_profileLocation;
    }

    @Override
    public void deploy() {
        super.deploy();
        String firmwareName = getSettings().getSetting(FIRMWARE_ROM_SETTING).getValue();
        if (firmwareName != null) {
            m_configuration.generate(firmwareName, getSpecification().getModelId());
            try {
                StringWriter writer = new StringWriter();
                m_configuration.write(writer, null);
                writer.close();
                BufferedWriter out = new BufferedWriter(new FileWriter(m_configuration.getPath()));
                out.write(writer.getBuffer().toString());
                out.close();
            } catch (IOException e) {
                LOG.error("KarelIP11xUpload deploy error " + e.getStackTrace());
            }
        }
    }

    @Override
    public void undeploy() {
        super.undeploy();
        String firmwareName = getSettings().getSetting(FIRMWARE_ROM_SETTING).getValue();
        if (firmwareName != null) {
            m_configuration.generate(firmwareName, getSpecification().getModelId());
            try {
                StringWriter writer = new StringWriter();
                m_configuration.write(writer, null);
                writer.close();
                File deviceFile = new File(m_configuration.getPath());
                if (deviceFile.exists()) {
                    deviceFile.delete();
                }
            } catch (IOException e) {
                LOG.error("KarelIP11xUpload undeploy error " + e.getStackTrace());
            }
        }
    }

}
