/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LdapImportPreview extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/ldap/LdapImportPreview";

    public abstract LdapImportManager getLdapImportManager();

    public abstract void setUser(User user);

    public abstract User getUser();

    public abstract void setGroupsString(String groups);

    public abstract int getIndex();

    public abstract void setIndex(int index);

    public abstract List<UserPreview> getExample();

    public abstract void setExample(List<UserPreview> example);

    public abstract void setFile(File file);

    public abstract File getFile();

    public abstract void setCurrentConnectionId(int connectionId);

    @Persist
    public abstract int getCurrentConnectionId();

    public void pageBeginRender(PageEvent event) {
        if (getUser() == null) {
            setUser(new User());
        }

        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator(getPage());
        try {
            if (getExample() == null) {
                List<UserPreview> example = getLdapImportManager().getExample(getCurrentConnectionId());
                setExample(example);
                File file = loadPreviewToFile(example);
                setFile(file);
            }

            if (!event.getRequestCycle().isRewinding()) {
                move(validator);
            }
        } catch (UserException e) {
            validator.record(new ValidatorException(e.getMessage()));
        }
    }

    private File loadPreviewToFile(List<UserPreview> list) {
        try {
            File file = getFile();
            if (file != null) {
                file.delete();
            }
            file = File.createTempFile("ldap", ".csv");
            // FIXME: it really should be deleted by the time session terminates
            file.deleteOnExit();
            FileWriter writer = new FileWriter(file);
            getLdapImportManager().dumpExample(list, writer, getCurrentConnectionId());
            writer.close();
            return file;
        } catch (IOException e) {
            throw new UserException(e.getMessage());
        }
    }

    private void move(SipxValidationDelegate validator) {
        List<UserPreview> example = getExample();
        if (example.isEmpty()) {
            throw new UserException(getMessages().getMessage("msg.empty"));
        }

        if (getIndex() >= example.size()) {
            setIndex(example.size() - 1);
        }
        if (getIndex() < 0) {
            setIndex(0);
        }

        UserPreview preview = example.get(getIndex());
        setUser(preview.getUser());
        Collection<String> groupNames = preview.getGroupNames();
        String groupsString = StringUtils.join(groupNames.iterator(), " ");
        setGroupsString(groupsString);

        validator.recordSuccess(getMessages().getMessage("msg.success"));
    }

    public void next() {
        int index = getIndex();
        setIndex(index + 1);
    }

    public void previous() {
        int index = getIndex();
        setIndex(index - 1);
    }

    public String ok() {
        setExample(null);
        setFile(null);
        return LdapPage.PAGE;
    }
}
