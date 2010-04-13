/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdProvisioningContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.LoggingManager;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxService;

public abstract class ManageLoggingLevels extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/ManageLoggingLevels";
    public static final String DEBUG = "DEBUG";

    private static final IPropertySelectionModel LOGGING_LEVEL_MODEL = new StringPropertySelectionModel(
            new String[] {
                DEBUG, "INFO", "NOTICE", "WARNING", "ERR", "CRIT", "ALERT", "EMERG"
            });

    private static final IPropertySelectionModel FREESWITCH_LOGGING_LEVEL_MODEL = new StringPropertySelectionModel(
            new String[] {
                DEBUG, "NON-DEBUG"
            });

    private String m_generalLevel;

    @Persist
    public abstract boolean isAdvanced();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:loggingManager")
    public abstract LoggingManager getLoggingManager();

    @InjectObject(value = "spring:serviceConfigurator")
    public abstract ServiceConfigurator getServiceConfigurator();

    @InjectObject(value = "spring:dialPlanActivationManager")
    public abstract DialPlanActivationManager getDialPlanActivationManager();

    @InjectObject(value = "spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject(value = "spring:acdContext")
    public abstract AcdContext getAcdContext();

    @InjectObject(value = "spring:acdProvisioningContext")
    public abstract AcdProvisioningContext getAcdProvisioningContext();

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract Collection<LoggingEntity> getLoggingEntities();

    public abstract void setLoggingEntities(Collection<LoggingEntity> entities);

    public abstract LoggingEntity getCurrentLoggingEntity();

    public abstract void setCurrentLoggingEntity(LoggingEntity entity);

    public IPropertySelectionModel getLoggingLevelsModel() {
        ExtraOptionModelDecorator model = new ExtraOptionModelDecorator();
        model.setModel(LOGGING_LEVEL_MODEL);
        model.setExtraLabel(getMessages().getMessage("option.selectLevel"));
        model.setExtraOption(null);
        return model;
    }

    public IPropertySelectionModel getFreeswitchLoggingLevelsModel() {
        return FREESWITCH_LOGGING_LEVEL_MODEL;
    }

    public String getGeneralLevel() {
        return m_generalLevel;
    }

    public void setGeneralLevel(String generalLevel) {
        m_generalLevel = generalLevel;
    }

    public void pageBeginRender(PageEvent event) {
        if (getLoggingEntities() == null) {
            Collection<LoggingEntity> entities = getLoggingManager().getLoggingEntities();
            setLoggingEntities(entities);
        }
        // just to make sure the general log level will not remain on some value and to override
        // the individual log levels
        setGeneralLevel(null);
        if (event.getRequestCycle().isRewinding()) {
            getLoggingManager().setEntitiesToProcess(new ArrayList<LoggingEntity>());
        }
    }

    public void commit() {
        List<SipxService> services = new ArrayList<SipxService>();
        Collection<LoggingEntity> entitiesToProcess;
        if (m_generalLevel != null) {
            entitiesToProcess = getLoggingEntities();
        } else {
            entitiesToProcess = getLoggingManager().getEntitiesToProcess();
        }
        for (LoggingEntity entity : entitiesToProcess) {
            if (m_generalLevel != null) {
                entity.setLogLevel(m_generalLevel);
            }

            SipxService serviceToRestart = getLoggingManager().getSipxServiceForLoggingEntity(entity);

            if (entity instanceof SipxService) {
                services.add((SipxService) entity);
            } else if (entity instanceof BridgeSbc) {
                SbcDeviceManager sbcDeviceContext = getSbcDeviceManager();
                sbcDeviceContext.storeSbcDevice((BridgeSbc) entity);
                getDialPlanActivationManager().replicateDialPlan(true);
                getServiceConfigurator().markServiceForRestart(serviceToRestart);
            } else if (entity instanceof AcdServer) {
                AcdServer modelServer = (AcdServer) entity;
                AcdContext context = getAcdContext();
                List<AcdServer> servers = context.getServers();
                for (AcdServer acdServer : servers) {
                    acdServer.setSettingValue(AcdServer.LOG_SETTING, modelServer
                            .getSettingValue(AcdServer.LOG_SETTING));
                    context.store(acdServer);
                    getAcdProvisioningContext().deploy(acdServer.getId());
                }
            }
        }
        getServiceConfigurator().replicateServiceConfig(services);

        getLoggingManager().setEntitiesToProcess(new ArrayList<LoggingEntity>());

        //forces page refresh
        setLoggingEntities(null);
    }
}
