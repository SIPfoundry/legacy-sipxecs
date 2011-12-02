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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;
import org.sipfoundry.sipxconfig.common.UserException;

public class FtpContextImpl implements FtpContext {
    private static final Log LOG = LogFactory.getLog(FtpContextImpl.class);

    private static final String UP = "..";
    private static final String INTERNAL_FTP_ERROR = "&message.ftpServerError";

    private FTPClient m_client;

    private String m_host;
    private String m_userId;
    private String m_password;

    public void openConnection() {
        if (m_client != null) {
            return;
        }
        m_client = new FTPClient();
        try {
            m_client.connect(m_host);
            int reply = m_client.getReplyCode();

            // After connection attempt, you should check the reply code to verify
            // success.
            if (!FTPReply.isPositiveCompletion(reply)) {
                closeConnection();
                throw new UserException("&message.refusedConnection");
            }
            if (!m_client.login(m_userId, m_password)) {
                closeConnection();
                throw new UserException("&message.userPass");
            }
            m_client.setFileType(FTP.BINARY_FILE_TYPE);
            m_client.enterLocalPassiveMode();
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException("&message.notConnect");
        }
    }

    private void makeDirectory(String directoryName) {
        try {
            m_client.makeDirectory(directoryName);
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    private String[] listByType(String path, Integer type) {
        List<String> names = new ArrayList<String>();
        try {
            FTPFile[] files = m_client.listFiles(path);
            for (FTPFile file : files) {
                if (file == null) {
                    continue;
                }
                if (type == null || type.intValue() == file.getType()) {
                    names.add(file.getName());
                }
            }
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
        return names.toArray(new String[names.size()]);
    }

    public String[] listDirectories(String path) {
        return listByType(path, new Integer(FTPFile.DIRECTORY_TYPE));
    }

    public String[] listFiles(String path) {
        return listByType(path, new Integer(FTPFile.FILE_TYPE));
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
                    changeDirectory(UP);
                }
            }
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    public void changeDirectory(String directory) {
        try {
            m_client.changeWorkingDirectory(directory);
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    private FTPFile[] searchFiles(String... names) {
        try {
            FTPFile[] allFiles = m_client.listFiles(".");
            List<FTPFile> filesToSearch = new ArrayList<FTPFile>();
            for (FTPFile file : allFiles) {
                if (ArrayUtils.contains(names, file.getName())) {
                    filesToSearch.add(file);
                }
            }

            return filesToSearch.toArray(new FTPFile[0]);
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    public void deleteDirectory(String directory) {
        FTPFile[] ftpDirectory = searchFiles(directory);
        delete(ftpDirectory);
    }

    private void delete(FTPFile... files) {
        try {
            for (FTPFile file : files) {
                if (file.isFile()) {
                    m_client.deleteFile(file.getName());
                } else if (file.isDirectory()) {
                    changeDirectory(file.getName());
                    delete(m_client.listFiles());
                    changeDirectory(UP);
                    m_client.removeDirectory(file.getName());
                }
            }

        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    public void download(String locationPath, String... names) {
        download(locationPath, searchFiles(names));
    }

    private void download(String locationPath, FTPFile... files) {
        try {
            for (FTPFile file : files) {
                if (file.isFile()) {
                    OutputStream output = new FileOutputStream(locationPath + File.separator + file.getName());
                    m_client.retrieveFile(file.getName(), output);
                    output.close();
                } else if (file.isDirectory()) {
                    changeDirectory(file.getName());
                    File fileNew = new File(locationPath + File.separator + file.getName());
                    fileNew.mkdir();
                    download(locationPath + File.separator + file.getName(), m_client.listFiles());
                    changeDirectory(UP);
                }
            }

        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        }
    }

    public void closeConnection() {
        if (m_client == null || !m_client.isConnected()) {
            return;
        }
        try {
            m_client.logout();
            m_client.disconnect();
        } catch (IOException e) {
            LOG.error(e);
            throw new UserException(INTERNAL_FTP_ERROR);
        } finally {
            m_client = null;
        }
    }

    public void setHost(String host) {
        m_host = host;
    }

    public void setPassword(String password) {
        m_password = password;
    }

    public void setUserId(String userId) {
        m_userId = userId;
    }
}
