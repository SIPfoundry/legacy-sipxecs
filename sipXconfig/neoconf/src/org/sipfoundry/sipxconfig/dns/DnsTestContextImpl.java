/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dns;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class DnsTestContextImpl implements DnsTestContext {
    private static final Log LOG = LogFactory.getLog(DnsTestContextImpl.class);
    private String m_validatorScript;

    @Override
    public String missingRecords(String server) {
        File missingRecords = null;
        Reader rdr = null;
        String commandLine = StringUtils.EMPTY;
        try {
            missingRecords = File.createTempFile("dns-test", ".tmp");
            String[] cmd = new String[] {
                m_validatorScript,
                "--server",
                server,
                "--out",
                missingRecords.getAbsolutePath()
            };
            LOG.info(StringUtils.join(cmd, ' '));
            ProcessBuilder pb = new ProcessBuilder(cmd);
            Process process = pb.start();
            int code = process.waitFor();
            if (code != 0) {
                String errorMsg = String.format("DNS validation command %s failed. Exit code: %d", commandLine, code);
                throw new RuntimeException(errorMsg);
            }
            rdr = new FileReader(missingRecords);
            return IOUtils.toString(rdr);
        } catch (IOException e) {
            String errorMsg = String.format("Error running archive command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        } catch (InterruptedException e) {
            String errorMsg = String.format("Timed out running archive command %s.", commandLine);
            throw new RuntimeException(errorMsg);
        } finally {
            IOUtils.closeQuietly(rdr);
            if (missingRecords != null) {
                missingRecords.delete();
            }
        }
    }

    public void setValidatorScript(String validatorScript) {
        m_validatorScript = validatorScript;
    }
}
