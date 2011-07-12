/*
 *
 *
 * Copyright (C) 2011 Karel Electronics Corp.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import java.util.Properties;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class KarelIP11xModelConfiguration extends TemplateConfigurationFile {

    private String m_firmwareName;
    private DomainManager m_domainManager;
    private LocationsManager m_locationsManager;
    private Properties m_model2ConfigFile;
    private Properties m_firmware2Model;

    public void setFirmwareName(String firmwareName) {
        m_firmwareName = firmwareName;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setModel2ConfigFile(Properties model2ConfigFile) {
        m_model2ConfigFile = model2ConfigFile;
    }

    public void setFirmware2Model(Properties firmware2Model) {
        m_firmware2Model = firmware2Model;
    }

    public Properties getModel2ConfigFile() {
        return m_model2ConfigFile;
    }

    public Properties getFirmware2Model() {
        return m_firmware2Model;
    }

    /**
     * Sets the configuration file name generated from the firmware name. Firmware name format is
     * IP11x_*.rom, model name is the same as first 5 characters.
     *
     * @param firmwareName
     */
    public void generate(String firmwareName, String modelId) {
        String phoneModel = firmwareName.substring(0, 5);
        String configFileName = m_model2ConfigFile.getProperty(phoneModel);
        String phoneModelId = m_firmware2Model.getProperty(modelId);
        if (configFileName != null && phoneModelId.compareTo(phoneModel) == 0) {
            setName(configFileName);
            m_firmwareName = firmwareName;
            return;
        }
        throw new UserException("&error.wrongFirmwareName", firmwareName, phoneModelId);
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("firmwareName", m_firmwareName);
        context.put("domainName", m_domainManager.getDomain().getName());
        context.put("fqdn", m_locationsManager.getPrimaryLocation().getFqdn());
        return context;
    }
}
