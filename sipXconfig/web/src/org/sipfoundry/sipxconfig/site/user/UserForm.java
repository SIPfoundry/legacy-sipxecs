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
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class UserForm extends BaseComponent implements EditPinComponent {
    public abstract CoreContext getCoreContext();

    public abstract SettingDao getSettingDao();

    public abstract ExtensionPoolContext getExtensionPoolContext();

    @Parameter(required = true)
    public abstract User getUser();

    public abstract String getPin();

    public abstract void setPin(String pin);

    public abstract String getAliasesString();

    public abstract void setAliasesString(String aliasesString);

    public abstract String getGroupsString();

    public abstract void setGroupsString(String groups);

    @Parameter(required = true)
    public abstract List<Group> getAvailableGroups();

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

            initializePin(getComponent("pin"), this, getUser());

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

            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, groupsString, false);
                user.setGroupsAsList(groups);
            }
        }
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
        String dup = getCoreContext().checkForDuplicateNameOrAlias(getUser());
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
        if (!user.isNew() && editPin.getPin() == null) {
            editPin.setPin(DUMMY_PIN);

            // Reset the confirm PIN field as well. Ugly to reach into the component
            // like this, but I haven't figured out a better way.
            PropertyUtils.write(confirmPassword, "confirmPassword", DUMMY_PIN);
        }
    }

    /*
     * Update the user's PIN
     */
    public static void updatePin(EditPinComponent editPin, User user, String authorizationRealm) {
        if (!(editPin.getPin() == null) && !editPin.getPin().equals(DUMMY_PIN)) {
            user.setPin(editPin.getPin(), authorizationRealm);

            // Having updated the user, scrub the PIN field for security
            editPin.setPin(null);
        }
    }

    private void recordError(String messageId, String arg) {
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());

        String message = MessageFormat.format(getMessages().getMessage(messageId), new Object[] {
            arg
        });

        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }
}
