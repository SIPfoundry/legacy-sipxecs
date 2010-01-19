/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class PhonebookFileEntryTrigger extends InitTaskListener {
    private static final Log LOG = LogFactory.getLog(PhonebookFileEntryTrigger.class);

    private PhonebookManager m_phonebookManager;

    public void setPhonebookManager(PhonebookManager phonebookManager) {
        m_phonebookManager = phonebookManager;
    }

    @Override
    public void onInitTask(String task) {
        LOG.info("Begining saving entries from existing CSV/vCard files into the table phonebook_file_entry.");

        String directory = m_phonebookManager.getExternalUsersDirectory();
        File dir = new File(directory);

        if (dir.exists()) {
            Map<Integer, String[]> files = m_phonebookManager.getPhonebookFilesName();

            for (Map.Entry<Integer, String[]> entry : files.entrySet()) {
                Integer phonebookId = entry.getKey();
                for (String fileName : entry.getValue()) {
                    if (StringUtils.isBlank(fileName)) {
                        continue;
                    }
                    try {
                        File f = new File(directory, fileName);
                        InputStream fileStream = new FileInputStream(f);
                        m_phonebookManager.addEntriesFromFile(phonebookId, fileStream);
                        f.delete();
                    } catch (FileNotFoundException e) {
                        LOG.warn("File not found error :" + e.getMessage());
                    }
                }
            }

            m_phonebookManager.removeTableColumns();

            if (dir.delete()) {
                LOG.info("Directory " + directory + "was successfully deleted !");
            } else {
                LOG.info("Failed to delete directory " + directory);
            }

        } else {
            LOG.info("Phonebook's directory " + directory + " doesn't exist!");
        }
    }
}
