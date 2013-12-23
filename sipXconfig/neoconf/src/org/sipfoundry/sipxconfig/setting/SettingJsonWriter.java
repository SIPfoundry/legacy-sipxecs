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
package org.sipfoundry.sipxconfig.setting;

import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.codehaus.jackson.JsonGenerator;
import org.codehaus.jackson.JsonProcessingException;
import org.codehaus.jackson.map.JsonSerializer;
import org.codehaus.jackson.map.SerializerProvider;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;

public class SettingJsonWriter extends JsonSerializer<Setting> {
    private static final String TYPE = "type";
    private static final String VALUE = "value";
    private static final Map<String, String> GROUP_TYPE = new HashMap<String, String>();
    private Locale m_locale;

    static {
        GROUP_TYPE.put("name", "group");
    }

    public SettingJsonWriter(Locale locale) {
        m_locale = locale;
    }

    public Locale getLocale() {
        return m_locale;
    }

    public void setLocale(Locale locale) {
        m_locale = locale;
    }

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
