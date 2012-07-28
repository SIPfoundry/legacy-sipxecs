/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import java.text.MessageFormat;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.commons.userdb.profile.Salutation;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.NewEnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserForm extends BaseComponent implements EditPinComponent {

    private static final String CONFIRM_PASSWORD = "confirmPassword";
    private static final String EMPTY = "empty";
    private static final String EMPTY_PIN = "";
    private static final String PIN = "pin";
    private static final String VOICEMAIL_PIN = "voicemail_pin";
    private static final String RENDER = "render";

    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingDao();

    public abstract ExtensionPoolContext getExtensionPoolContext();

    @Parameter(required = true)
    public abstract User getUser();

    public abstract String getPin();

    public abstract void setPin(String pin);

    public abstract String getVoicemalPin();

    public abstract void setVoicemailPin(String voicemailPin);

    public abstract String getAliasesString();

    public abstract void setAliasesString(String aliasesString);

    public abstract String getGroupsString();

    public abstract void setGroupsString(String groups);

    public abstract void setSalutationModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getSalutationModel();

    public void prepareForRender(IRequestCycle cycle) {
        UserProfile userProfile = getUser().getUserProfile();
        if (userProfile == null) {
            userProfile = new UserProfile();
            getUser().setUserProfile(userProfile);
        }
        if (getSalutationModel() == null) {
            NewEnumPropertySelectionModel model = new NewEnumPropertySelectionModel();
            model.setEnumType(Salutation.class);
            setSalutationModel(model);
        }
    }

    @EventListener(events = "onclick", targets = EMPTY)
    public void emptyPasswords(IRequestCycle cycle) {
        setPin(EMPTY_PIN);
        setVoicemailPin(EMPTY_PIN);
        PropertyUtils.write(getComponent(PIN), CONFIRM_PASSWORD, EMPTY_PIN);
        PropertyUtils.write(getComponent(VOICEMAIL_PIN), CONFIRM_PASSWORD, EMPTY_PIN);
        cycle.getResponseBuilder().updateComponent(RENDER);
    }

    // Update the User object if input data is valid
    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {

        if (!TapestryUtils.isRewinding(cycle, this)) {
            User user = getUser();
            // Automatically assign a numeric extension if appropriate
            assignExtension();

            // Init the aliases string before rendering, if necessary
            if (StringUtils.isEmpty(getAliasesString())) {
                setAliasesString(user.getAliasesString());
            }

            initializePin(getComponent(PIN), this, getUser());
            initializeVoicemailPin(getComponent(VOICEMAIL_PIN), this, getUser());

            if (getGroupsString() == null) {
                setGroupsString(user.getGroupsNames());
            }
        }

        super.renderComponent(writer, cycle);

        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            User user = getUser();

            // Set the user aliases from the aliases string
            user.setAliasesString(getAliasesString());
            // XCF-1243 clean alias string on the screen
            setAliasesString(null);

            // Make sure that the user ID and aliases don't collide with any other
            // user IDs or aliases. Report an error if there is a collision.
            if (checkForUserIdOrAliasCollision()) {
                return;
            }

            // Update the user's PIN and aliases
            updatePin(this, getUser(), getCoreContext().getAuthorizationRealm());
            updateVoicemailPin(this, getUser());

            String groupsString = getGroupsString();
            if (groupsString != null) {
                List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, groupsString, false);
                //Make sure that the actual group selection can be accepted
                if (groupsAvailable(groups)) {
                    user.setGroupsAsList(groups);
                }
            }
        }
    }

    private boolean groupsAvailable(List<Group> groups) {
        for (Group group : groups) {
            if (!getUser().isGroupAvailable(group)) {
                recordError("invalid.group", null);
                return false;
            }
        }
        return true;
    }

    // If the userName is empty and the user extension pool is enabled, then
    // try to fill in the userName with the next free extension from the pool.
    private void assignExtension() {
        if (!StringUtils.isEmpty(getUser().getUserName())) {
            return; // there is already a username, don't overwrite it
        }

        // Get and use the next free extension
        ExtensionPoolContext epc = getExtensionPoolContext();
        Integer extension = epc.getNextFreeUserExtension();
        if (extension != null) {
            String extStr = extension.toString();
            getUser().setUserName(extStr);
        }
    }

    // Make sure that the user ID and aliases don't collide with any other
    // user IDs or aliases. Report an error if there is a collision.
    private boolean checkForUserIdOrAliasCollision() {
        boolean result = false;
        String dup = null;
        try {
            dup = getCoreContext().checkForDuplicateNameOrAlias(getUser());
        } catch (Exception ex) {
            recordError("err.msg.checkUserIdAliasFailed", ex.getMessage());
            return result;
        }
        if (dup != null) {
            result = true;
            boolean internalCollision = false;

            // Check for a collision within the user itself, of the user ID with an alias,
            // so we can give more specific error feedback. Since the aliases are filtered
            // for duplicates when assigned to the user, we don't have to worry about that
            // case. Duplicate aliases are simply discarded.
            for (Iterator iter = getUser().getAliases().iterator(); iter.hasNext();) {
                String alias = (String) iter.next();
                if (getUser().getUserName().equals(alias)) {
                    recordError("message.userIdEqualsAlias", alias);
                    internalCollision = true;
                    break;
                }
            }
            // If it wasn't an internal collision, then the collision is with a different
            // user. Record an appropriate error.
            if (!internalCollision) {
                recordError("message.duplicateUserIdOrAlias", dup);
            }
        }
        return result;
    }

    /**
     * For an existing user with a non-empty PIN, init the displayed PIN to be the dummy PIN to
     * make it clear that the PIN is not empty.
     */
    public static void initializePin(IComponent confirmPassword, EditPinComponent editPin, User user) {
        if (editPin.getPin() == null) {
            if (user.isNew()) {
                editPin.setPin(user.getPintoken());
                PropertyUtils.write(confirmPassword, CONFIRM_PASSWORD, user.getPintoken());
            } else {
                editPin.setPin(DUMMY_PIN);
                PropertyUtils.write(confirmPassword, CONFIRM_PASSWORD, DUMMY_PIN);
            }
        }
    }

    /**
     * For an existing user with a non-empty PIN, init the displayed PIN to be the dummy PIN to
     * make it clear that the PIN is not empty.
     */
    public static void initializeVoicemailPin(IComponent confirmPassword, EditPinComponent editPin, User user) {
        if (editPin.getVoicemailPin() == null) {
            if (user.isNew()) {
                editPin.setVoicemailPin(user.getClearVoicemailPin());
                PropertyUtils.write(confirmPassword, CONFIRM_PASSWORD, user.getClearVoicemailPin());
            } else {
                editPin.setVoicemailPin(DUMMY_PIN);
                PropertyUtils.write(confirmPassword, CONFIRM_PASSWORD, DUMMY_PIN);
            }
        }
    }

    /*
     * Update the user's PIN
     */
    public static void updatePin(EditPinComponent editPin, User user, String authorizationRealm) {
        if (!(editPin.getPin() == null) && !editPin.getPin().equals(DUMMY_PIN)) {
            user.setPin(editPin.getPin());

            // Having updated the user, scrub the PIN field for security
            editPin.setPin(null);
        }
    }

    public static void updateVoicemailPin(EditPinComponent editPin, User user) {
        if (!(editPin.getVoicemailPin() == null) && !editPin.getVoicemailPin().equals(DUMMY_PIN)) {
            user.setVoicemailPin(editPin.getVoicemailPin());

            // Having updated the user, scrub the PIN field for security
            editPin.setVoicemailPin(null);
        }
    }

    private void recordError(String messageId, String arg) {
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
        String message = null;
        if (arg != null) {
            message = MessageFormat.format(getMessages().getMessage(messageId), arg);
        } else {
            message = getMessages().getMessage(messageId);
        }

        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }
}
