/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.rest;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;

import java.io.Serializable;

import org.apache.commons.lang.StringUtils;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;

import com.thoughtworks.xstream.XStream;

public class ContactInformationResource extends UserResource {

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        User user = getUser();
        UserProfile userProfile = user.getUserProfile();

        boolean imEnabled = (Boolean) user.getSettingTypedValue("im/im-account");
        Representable representable = new Representable(user.getFirstName(), user.getLastName(), userProfile,
                imEnabled);

        return new UserProfileRepresentation(variant.getMediaType(), representable);
    }

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        UserProfileRepresentation representation = new UserProfileRepresentation(entity);
        Representable representable = representation.getObject();

        UserProfile reprUserProfile = new UserProfile();
        reprUserProfile.update(representable);

        User user = getUser();
        UserProfile userProfile = user.getUserProfile();

        if (userProfile == null) {
            user.setUserProfile(reprUserProfile);
        } else {
            //the IM id needs to be uneditable via rest.(XX-8022)
            reprUserProfile.setImId(userProfile.getImId());
            reprUserProfile.setSalutation(userProfile.getSalutation());
            reprUserProfile.setManager(userProfile.getManager());
            reprUserProfile.setEmployeeId(userProfile.getEmployeeId());
            userProfile.update(reprUserProfile);
            user.setUserProfile(userProfile);
        }
        user.setFirstName(representable.getFirstName());
        user.setLastName(representable.getLastName());

        getCoreContext().saveUser(user);
    }

    static class Representable extends UserProfile implements Serializable {
        private final String m_firstName;
        private final String m_lastName;

        public Representable(String firstName, String lastName, UserProfile userProfile, boolean imEnabled) {
            m_firstName = firstName;
            m_lastName = lastName;
            if (userProfile != null) {
                this.update(userProfile);
                if (!imEnabled) {
                    setImId(StringUtils.EMPTY);
                }

            }
        }

        public String getFirstName() {
            return m_firstName;
        }

        public String getLastName() {
            return m_lastName;
        }
    }

    static class UserProfileRepresentation extends XStreamRepresentation<Representable> {
        public UserProfileRepresentation(MediaType mediaType, Representable object) {
            super(mediaType, object);
        }

        public UserProfileRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.omitField(BeanWithId.class, "m_id");
            xstream.omitField(UserProfile.class, "m_userName");
            xstream.omitField(UserProfile.class, "m_userid");
            xstream.omitField(UserProfile.class, "m_firstName");
            xstream.omitField(UserProfile.class, "m_lastName");
            xstream.omitField(UserProfile.class, "m_useExtAvatar");
            xstream.omitField(UserProfile.class, "m_extAvatar");
            xstream.alias("contact-information", Representable.class);
            xstream.aliasField("avatar", Representable.class, "m_avatar");
        }
    }
}
