/*
 *
 *  Copyright (C) 2012 PATLive, D. Waseem, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdSettingsResource.java - A Restlet to read Skill data from OpenACD within SipXecs
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.

 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package org.sipfoundry.sipxconfig.rest;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_UPDATE_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.SUCCESS_UPDATED;

import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSettings;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdSettingsResource extends Resource {

    private static final String ELEMENT_NAME_SETTING = "setting";

    private OpenAcdContext m_openAcdContext;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public boolean allowDelete() {
        return false;
    }

    // GET - Retrieve all Settings
    // ---------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        // process request for list
        OpenAcdSettings settings = m_openAcdContext.getSettings();
        OpenAcdSettingRestInfo settingsRestInfo = new OpenAcdSettingRestInfo(settings);

        return new OpenAcdSettingRepresentation(variant.getMediaType(), settingsRestInfo);
    }

    // PUT - Update Settings
    // ---------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        // get from request body
        OpenAcdSettingRepresentation representation = new OpenAcdSettingRepresentation(entity);
        OpenAcdSettingRestInfo settingRestInfo = representation.getObject();
        OpenAcdSettings settings;

        // assign new setting
        try {
            settings = m_openAcdContext.getSettings();
            updateSettings(settings, settingRestInfo);
            m_openAcdContext.saveSettings(settings);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, settings.getId());
    }

    // Helper functions
    // ----------------

    private void updateSettings(OpenAcdSettings settings, OpenAcdSettingRestInfo settingRestInfo)
        throws ResourceException {
        settings.setSettingValue("openacd-config/log_level", settingRestInfo.getLogLevel());
    }

    // REST Representations
    // --------------------

    static class OpenAcdSettingRepresentation extends XStreamRepresentation<OpenAcdSettingRestInfo> {

        public OpenAcdSettingRepresentation(MediaType mediaType, OpenAcdSettingRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdSettingRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_SETTING, OpenAcdSettingRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdSettingRestInfo {
        private final int m_id;
        private final String m_logLevel;

        public OpenAcdSettingRestInfo(OpenAcdSettings setting) {
            m_id = setting.getId();
            m_logLevel = setting.getLogLevel();
        }

        public int getId() {
            return m_id;
        }

        public String getLogLevel() {
            return m_logLevel;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
