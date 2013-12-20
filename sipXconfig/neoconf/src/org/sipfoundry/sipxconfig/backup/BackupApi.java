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
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.codehaus.jackson.JsonGenerator;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonProcessingException;
import org.codehaus.jackson.Version;
import org.codehaus.jackson.map.JsonSerializer;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.map.SerializerProvider;
import org.codehaus.jackson.map.module.SimpleModule;
import org.restlet.Context;
import org.restlet.data.ClientInfo;
import org.restlet.data.Language;
import org.restlet.data.Preference;
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
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;

public class BackupApi extends Resource {
    private static final String TYPE = "type";
    private static final String VALUE = "value";
    private static final Map<String, String> GROUP_TYPE = new HashMap<String, String>();
    private BackupManager m_backupManager;
    private BackupType m_backupType;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private Locale m_locale;
    private MessageSource m_messages;
    private BackupPlan m_plan;
    private BackupSettings m_settings;
    private ManualBackup m_manualBackup;

    static {
        GROUP_TYPE.put("name", "group");
    }

    public BackupApi() {
        SimpleModule testModule = new SimpleModule("MyModule", new Version(1, 0, 0, null));
        testModule.addSerializer(new SettingSerializer());
        m_jsonMapper.registerModule(testModule);
    }

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String type = (String) getRequest().getAttributes().get(TYPE);
        if (type != null) {
            m_backupType = BackupType.valueOf(type);
        }
        m_locale = getLocale(request);
    }

    static Locale getLocale(Request request) {
        ClientInfo ci = request.getClientInfo();
        if (ci != null) {
            List<Preference<Language>> langs = ci.getAcceptedLanguages();
            if (langs != null && langs.size() > 0) {
                Language lmeta = langs.get(0).getMetadata();
                if (lmeta != null && lmeta.getName() != null) {
                    // Java 1.7 only
                    //   Locale.forLanguageTag(lmeta.getName());
                    return forLanguageTag(lmeta.getName());
                }
            }
        }
        return Locale.ENGLISH;
    }

    static Locale forLanguageTag(String id) {
        String[] segments = StringUtils.split(id, '-');
        switch (segments.length) {
        case 3:
            return new Locale(segments[0], segments[1], segments[2]);
        case 2:
            return new Locale(segments[0], segments[1]);
        default:
        case 1:
            return new Locale(id);
        }
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
            BackupCommandRunner runner = new BackupCommandRunner(planFile, m_backupManager.getBackupScript());
            runner.setMode(m_backupType.name());
            List<String> backups = runner.list();
            writeBackup(json, backup, backups, settings, archiveIdMap);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    void toJson(Writer json, Setting settings) throws IOException {
        m_jsonMapper.writeValue(json, settings);
    }

    class SettingSerializer extends JsonSerializer<Setting> {

        @Override
        public void serialize(Setting setting, JsonGenerator jg, SerializerProvider arg2) throws IOException,
            JsonProcessingException {

            String name = setting.getName();
            if (setting.isLeaf()) {
                jg.writeObjectFieldStart(name);
                jg.writeStringField(VALUE, setting.getValue());
                jg.writeStringField("default", setting.getDefaultValue());
                jg.writeObjectField(TYPE, setting.getType());
                serilizeCommon(setting, jg);
                jg.writeEndObject();
            } else {
                if (StringUtils.isBlank(name)) {
                    jg.writeStartObject();
                } else {
                    jg.writeObjectFieldStart(setting.getName());
                }
                jg.writeObjectField(TYPE, GROUP_TYPE);
                serilizeCommon(setting, jg);
                jg.writeObjectFieldStart(VALUE);
                Collection<Setting> children = setting.getValues();
                for (Setting child : children) {
                    serialize(child, jg, arg2);
                }
                jg.writeEndObject();
                jg.writeEndObject();
            }
        }

        void serilizeCommon(Setting setting, JsonGenerator jg) throws IOException {
            MessageSource messageSource = setting.getMessageSource();
            jg.writeObjectField("label", message(messageSource, setting.getLabelKey()));
            jg.writeObjectField("description", message(messageSource, setting.getDescriptionKey()));
        }

        String message(MessageSource ms, String key) {
            try {
                return ms.getMessage(key, null, m_locale);
            } catch (NoSuchMessageException e) {
                return null;
            }
        }

        @Override
        public Class<Setting> handledType() {
            return Setting.class;
        }
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

    void writeBackup(Writer json, BackupPlan plan, List<String> backups, BackupSettings settings,
        Map<String, String> archiveIds) throws IOException {

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
            plan.getAutoModeDefinitionIds());
        plan.getAutoModeDefinitionIds().removeAll(invalidOrOff);

        json.write("{\"definitions\":");
        m_jsonMapper.writeValue(json, archiveIds);
        json.write(",\"settings\":");
        m_jsonMapper.writeValue(json, settings.getSettings());
        json.write(",\"backup\":");
        m_jsonMapper.writeValue(json, plan);
        json.write(",\"backups\":");
        m_jsonMapper.writeValue(json, backups);
        json.write("}");
    }

    /**
     * PUT Save: yes Backup Now: no
     */
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        putOrPost(entity);
        m_backupManager.saveBackupPlan(m_plan);
        m_backupManager.saveSettings(m_settings);
    }

    /**
     * POST Save: no Backup Now: yes
     */
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        putOrPost(entity);
        m_plan.setType(BackupType.manual);
        m_manualBackup.backup(m_plan, m_settings);
    }

    void putOrPost(Representation entity) throws ResourceException {
        try {
            JsonNode meta = m_jsonMapper.readTree(entity.getReader());
            m_plan = m_backupManager.findOrCreateBackupPlan(m_backupType);
            readPlan(m_plan, meta.get("backup"));
            m_settings = m_backupManager.getSettings();
            updateSettings(m_settings, meta.get("settings"));
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    void updateSettings(PersistableSettings settings, JsonNode node) {
        Iterator<Entry<String, JsonNode>> fields = node.getFields();
        while (fields.hasNext()) {
            Entry<String, JsonNode> setting = fields.next();
            settings.setSettingValue(setting.getKey(), setting.getValue().asText());
        }
    }

    void readPlan(BackupPlan plan, JsonNode node) {
        Set<String> ids = new HashSet<String>();
        Iterator<JsonNode> nIds = node.get("autoModeDefinitionIds").getElements();
        while (nIds.hasNext()) {
            ids.add(nIds.next().asText());
        }
        plan.setAutoModeDefinitionIds(ids);
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
    public void setManualBackup(ManualBackup manualBackup) {
        m_manualBackup = manualBackup;
    }
}
