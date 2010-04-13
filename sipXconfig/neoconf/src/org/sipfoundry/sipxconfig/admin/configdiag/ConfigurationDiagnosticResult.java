/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.Serializable;

public class ConfigurationDiagnosticResult implements Serializable {
    public enum Status {
        Success, Warning, Error, Fatal, Unknown, InProgress;

        public static boolean hasDetails(Status status) {
            return status == Warning || status == Error || status == Fatal;
        }
    }

    public static final ConfigurationDiagnosticResult INITIAL_RESULT = new InitialTestResult(
            Integer.MIN_VALUE);
    public static final ConfigurationDiagnosticResult SUCCESS_RESULT = new SuccessTestResult();
    public static final ConfigurationDiagnosticResult INPROGRESS_RESULT = new InProgressTestResult();

    private Status m_status;
    private String m_message;
    private String m_description;
    private int m_exitStatus;

    public Status getStatus() {
        return m_status;
    }

    public void setStatus(Status status) {
        m_status = status;
    }

    public void setStatusAsString(String status) {
        m_status = Status.valueOf(status);
    }

    public String getMessage() {
        return m_message;
    }

    public void setMessage(String message) {
        m_message = message;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public int getExitStatus() {
        return m_exitStatus;
    }

    public void setExitStatus(int exitStatus) {
        m_exitStatus = exitStatus;
    }

    static UnknownTestResult createUnknown(int exitStatus) {
        return new UnknownTestResult(exitStatus);
    }

    private static class InitialTestResult extends ConfigurationDiagnosticResult {
        public InitialTestResult(int exitCode) {
            setExitStatus(exitCode);
            setStatus(Status.Unknown);
            setMessage("Test has not been run: " + exitCode);
        }
    }

    private static class UnknownTestResult extends ConfigurationDiagnosticResult {
        public UnknownTestResult(int exitCode) {
            setExitStatus(exitCode);
            setStatus(Status.Warning);
            setMessage("Unknown test result: " + exitCode);
        }
    }

    private static class SuccessTestResult extends ConfigurationDiagnosticResult {
        public SuccessTestResult() {
            setExitStatus(0);
            setStatus(Status.Success);
            setMessage("Success");
        }
    }

    private static class InProgressTestResult extends ConfigurationDiagnosticResult {
        public InProgressTestResult() {
            setExitStatus(0);
            setStatus(Status.InProgress);
            setMessage("InProgress");
        }
    }
}
