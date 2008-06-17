/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.ftp;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;

public class FtpContextImpl implements Serializable, FtpContext {
    public static final String TWO_POINTS = "..";
    private FTPClient m_client;

    private String m_host;
    private String m_userId;
    private String m_password;

    public void openConnection() {
        if (m_client == null) {
            m_client = new FTPClient();
            try {
                int reply;
                m_client.connect(m_host);
                reply = m_client.getReplyCode();

                // After connection attempt, you should check the reply code to verify
                // success.
                if (!FTPReply.isPositiveCompletion(reply)) {
                    closeConnection();
                    throw new UserException(false, "message.refusedConnection");
                }
                if (!m_client.login(m_userId, m_password)) {
                    closeConnection();
                    throw new UserException(false, "message.userPass");
                }
                m_client.setFileType(FTP.BINARY_FILE_TYPE);
                m_client.enterLocalPassiveMode();
            } catch (IOException e) {
                closeConnection();
                throw new UserException(false, "message.notConnect");
            }
        }

    }

    public void makeDirectory(String directoryName) {
        try {
            m_client.makeDirectory(directoryName);
        } catch (IOException e) {
            throw new UserException(e);
        }
    }

    public String[] list(String path) {
        String[] names;
        try {
            FTPFile[] files = m_client.listFiles(path);
            names = new String[files.length];
            int i = 0;
            for (FTPFile file : files) {
                names[i++] = file.getName();
            }

        } catch (IOException ex) {
            throw new UserException(ex);
        }
        return names;
    }

    public void upload(String... names) {
        InputStream input = null;
        File file = null;
        File[] filesChild = null;

        String[] fileNamesChild = null;
        try {
            for (String path : names) {
                file = new File(path);
                if (file.isFile()) {
                    input = new FileInputStream(file);
                    m_client.storeFile(file.getName(), input);
                    input.close();
                } else if (file.isDirectory()) {
                    makeDirectory(file.getName());
                    changeDirectory(file.getName());
                    filesChild = file.listFiles();
                    fileNamesChild = new String[filesChild.length];
                    int i = 0;
                    for (File fileChild : filesChild) {
                        fileNamesChild[i++] = fileChild.getPath();
                    }
                    upload(fileNamesChild);
                    changeDirectory(TWO_POINTS);
                }
            }
        } catch (FileNotFoundException fne) {
            throw new UserException(fne);
        } catch (IOException io) {
            throw new UserException(io);
        }
    }

    public void changeDirectory(String directory) {
        try {
            m_client.changeWorkingDirectory(directory);
        } catch (Exception ex) {
            throw new UserException(ex);
        }
    }

    private FTPFile[] searchFiles(String... names) {
        try {
            FTPFile[] allFiles = m_client.listFiles(".");
            List<FTPFile> filesToSearch = new ArrayList<FTPFile>();
            List<String> namesList = Arrays.asList(names);
            for (FTPFile file : allFiles) {
                if (namesList.contains(file.getName())) {
                    filesToSearch.add(file);
                }
            }

            return filesToSearch.toArray(new FTPFile[0]);
        } catch (IOException ex) {
            throw new UserException(ex);
        }
    }

    public void delete(String... names) {
        delete(searchFiles(names));
    }

    private void delete(FTPFile... files) {
        try {
            for (FTPFile file : files) {
                if (file.isFile()) {
                    m_client.deleteFile(file.getName());
                } else if (file.isDirectory()) {
                    changeDirectory(file.getName());
                    delete(m_client.listFiles());
                    changeDirectory(TWO_POINTS);
                    m_client.removeDirectory(file.getName());
                }
            }

        } catch (IOException ex) {
            throw new UserException(ex);
        }
    }

    public void download(String locationPath, String... names) {
        download(locationPath, searchFiles(names));
    }

    private void download(String locationPath, FTPFile... files) {
        OutputStream output;
        File fileNew;
        try {
            for (FTPFile file : files) {
                if (file.isFile()) {
                    output = new FileOutputStream(locationPath + File.separator + file.getName());
                    m_client.retrieveFile(file.getName(), output);
                    output.close();
                } else if (file.isDirectory()) {
                    changeDirectory(file.getName());
                    fileNew = new File(locationPath + File.separator + file.getName());
                    fileNew.mkdir();
                    download(locationPath + File.separator + file.getName(), m_client.listFiles());
                    changeDirectory(TWO_POINTS);
                }
            }

        } catch (IOException ex) {
            throw new UserException(ex);
        }
    }

    public void closeConnection() {
        if (m_client != null && m_client.isConnected()) {
            try {
                m_client.logout();
                m_client.disconnect();
            } catch (IOException f) {
                throw new UserException(f);
            }
        }
        m_client = null;
    }

    public String getHost() {
        return m_host;
    }

    @Required
    public void setHost(String host) {
        this.m_host = host;
    }

    public String getPassword() {
        return m_password;
    }

    @Required
    public void setPassword(String password) {
        this.m_password = password;
    }

    public String getUserId() {
        return m_userId;
    }

    @Required
    public void setUserId(String userId) {
        this.m_userId = userId;
    }

}
