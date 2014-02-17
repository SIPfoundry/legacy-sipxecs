/**
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
package org.sipfoundry.sipxconfig.backup;


import static java.lang.String.format;
import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.poi.util.TempFile;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.Version;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.map.module.SimpleModule;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.common.TimeOfDay;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.rest.RestUtilities;
import org.sipfoundry.sipxconfig.security.StandardUserDetailsService;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingJsonReader;
import org.sipfoundry.sipxconfig.setting.SettingJsonWriter;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.MessageSource;

public class BackupApi extends Resource {
    private static final Log LOG = LogFactory.getLog(BackupApi.class);

    private static final String BACKUP = "backup";
    private BackupManager m_backupManager;
    private LocationsManager m_locationsManager;
    private BackupType m_backupType;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private Locale m_locale;
    private MessageSource m_messages;
    private BackupPlan m_plan;
    private BackupSettings m_settings;
    private BackupConfig m_backupConfig;
    private BackupRunner m_backupRunner;
    private SettingJsonWriter m_settingWriter;

    public BackupApi() {
        SimpleModule testModule = new SimpleModule("MyModule", new Version(1, 0, 0, null));
        m_settingWriter = new SettingJsonWriter(m_locale);
        testModule.addSerializer(m_settingWriter);
        m_jsonMapper.registerModule(testModule);
    }

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String type = (String) getRequest().getAttributes().get("type");
        if (type != null) {
            m_backupType = BackupType.valueOf(type);
        }
        m_locale = RestUtilities.getLocale(request);
        m_settingWriter.setLocale(m_locale);
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        if (m_backupType == null) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, "Must specify type of backup /backup/{type}");
        }
        StringWriter json = new StringWriter();
        try {
            Map<String, String> archiveIdMap = getArchiveIdMap();
            BackupPlan backup = m_backupManager.findOrCreateBackupPlan(m_backupType);
            BackupSettings settings = m_backupManager.getSettings();
            File planFile = m_backupManager.getPlanFile(backup);
            Map<String, List<String>> backups = new HashMap<String, List<String>>();
            try {
                backups = m_backupRunner.list(planFile);
            } catch (Exception ex) {
                LOG.error("Cannot retrieve backups list ", ex);
            }
            UserDetailsImpl currentUser = StandardUserDetailsService.getUserDetails();
            Map<String, List<String>> hyperlinkedList = new TreeMap<String, List<String>>();
            for (Map.Entry<String, List<String>> entries : backups.entrySet()) {
                List<String> linkedEntries = new ArrayList<String>(entries.getValue().size());
                for (String entry : entries.getValue()) {
                    // encode as "label|url" so that we can keep this from being yet another nested collection
                    String encode = entry + '|'
                        + settings.getLink(backup, currentUser.getUserId(), entries.getKey(), entry);
                    linkedEntries.add(encode);
                }
                hyperlinkedList.put(entries.getKey(), linkedEntries);
            }
            boolean inProgress = m_backupRunner.isInProgress();
            writeBackup(json, inProgress, backup, hyperlinkedList, settings, archiveIdMap);
        } catch (Exception e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    void toJson(Writer json, Setting settings) throws IOException {
        m_jsonMapper.writeValue(json, settings);
    }

    Map<String, String> getArchiveIdMap() {
        Collection<String> archiveIds = m_backupManager.getArchiveDefinitionIds();
        Map<String, String> map = new HashMap<String, String>(archiveIds.size());
        for (String id : archiveIds) {
            String rcId = format("archive.%s.label", id);
            map.put(id, m_messages.getMessage(rcId, null, m_locale));
        }
        return map;
    }

    void writeBackup(Writer json, boolean inProgress, BackupPlan plan, Map<String, List<String>> backups,
        BackupSettings settings, Map<String, String> archiveIds) throws IOException {

        // every plan has exactly 1 schedule
        if (plan.getSchedules().isEmpty()) {
            DailyBackupSchedule schedule = new DailyBackupSchedule();
            plan.addSchedule(schedule);
        }

        // NOTE: it's possible to have old IDs in there as features were disabled. one could argue
        // these ids should be cleared up on feature enable/disable but it's rather convenient to
        // keep ids
        // in there as features are re-enabled, they are automatically part of backup plan. Unless
        // they
        // come to the backup page and save before they re-enabled features, we clear them here.
        // However,
        // WYSIWYG overrules this convenience.
        Collection< ? > invalidOrOff = CollectionUtils.disjunction(archiveIds.keySet(),
            plan.getDefinitionIds());
        plan.getDefinitionIds().removeAll(invalidOrOff);

        json.write("{\"definitions\":");
        m_jsonMapper.writeValue(json, archiveIds);
        json.write(",\"settings\":");
        m_jsonMapper.writeValue(json, settings.getSettings());
        json.write(",\"backup\":");
        m_jsonMapper.writeValue(json, plan);
        json.write(",\"backups\":");
        m_jsonMapper.writeValue(json, backups);
        json.write(",\"inProgress\":");
        m_jsonMapper.writeValue(json, inProgress);
        json.write("}");
    }

    /**
     * PUT
     *  Save plan: yes
     *  Backup Now: no
     */
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        putOrPost(entity);
        m_backupManager.saveBackupPlan(m_plan);
        m_backupManager.saveSettings(m_settings);
    }

    /**
     * POST
     *  Save plan: no
     *  Backup Now: yes
     */
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        putOrPost(entity);
        File planFile = null;
        Writer planWtr = null;
        String configuration = StringUtils.EMPTY;
        try {
            planFile = TempFile.createTempFile(BACKUP, "yaml");
            planWtr = new FileWriter(planFile);
            Collection<Location> hosts = m_locationsManager.getLocationsList();
            m_backupConfig.writeConfig(planWtr, m_plan, hosts, m_settings);
            IOUtils.closeQuietly(planWtr);
            planWtr = null;
            configuration = FileUtils.readFileToString(planFile);
            if (m_backupRunner.backup(planFile)) {
                LOG.info("Backup SUCCEEDED for configuration: " + configuration);
            }
        } catch (Exception e) {
            LOG.error("Backup FAILED for configuration: " + configuration, e);
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } finally {
            IOUtils.closeQuietly(planWtr);
            if (planFile != null) {
                planFile.delete();
            }
        }
    }

    void putOrPost(Representation entity) throws ResourceException {
        try {
            JsonNode meta = m_jsonMapper.readTree(entity.getReader());
            m_plan = m_backupManager.findOrCreateBackupPlan(m_backupType);
            readPlan(m_plan, meta.get(BACKUP));
            m_settings = m_backupManager.getSettings();
            SettingJsonReader settingJsonReader = new SettingJsonReader();
            settingJsonReader.read(m_settings, meta.get("settings"));
            settingJsonReader.read(m_settings.getSettings().getSetting("ftp"), meta.get("ftpSettings"));
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    void readPlan(BackupPlan plan, JsonNode node) {
        Set<String> ids = new HashSet<String>();
        Iterator<JsonNode> nIds = node.get("definitionIds").getElements();
        while (nIds.hasNext()) {
            ids.add(nIds.next().asText());
        }
        plan.setDefinitionIds(ids);
        plan.setLimitedCount(node.get("limitedCount").asInt());
        Iterator<JsonNode> nSchedules = node.get("schedules").iterator();
        for (Iterator<DailyBackupSchedule> i = plan.getSchedules().iterator(); nSchedules.hasNext();) {
            JsonNode nSchedule = nSchedules.next();
            JsonNode nTime = nSchedule.get("timeOfDay");
            TimeOfDay time = new TimeOfDay(nTime.get("hrs").asInt(), nTime.get("min").asInt());
            JsonNode nDay = nSchedule.get("scheduledDay");
            ScheduledDay day = ScheduledDay.getScheduledDay(nDay.get("dayOfWeek").asInt());
            DailyBackupSchedule schedule;
            if (i.hasNext()) {
                schedule = i.next();
            } else {
                schedule = new DailyBackupSchedule();
                plan.addSchedule(schedule);
            }
            schedule.setTimeOfDay(time);
            schedule.setScheduledDay(day);
            schedule.setEnabled(nSchedule.get("enabled").asBoolean());
        }
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowDelete() {
        return true;
    };

    @Override
    public boolean allowPut() {
        return true;
    };

    @Override
    public boolean allowPost() {
        return true;
    };

    ObjectMapper getJsonMapper() {
        return m_jsonMapper;
    }

    @Required
    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }

    @Required
    public void setMessages(MessageSource messages) {
        m_messages = messages;
    }

    @Required
    public void setBackupConfig(BackupConfig backupConfig) {
        m_backupConfig = backupConfig;
    }

    @Required
    public void setBackupRunner(BackupRunner backupRunner) {
        m_backupRunner = backupRunner;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
