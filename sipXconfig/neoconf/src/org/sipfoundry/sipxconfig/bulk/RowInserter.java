/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk;

import java.io.Serializable;

import org.apache.commons.collections.Closure;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.csv.CsvRowInserter;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;
import org.springframework.transaction.PlatformTransactionManager;
import org.springframework.transaction.TransactionStatus;
import org.springframework.transaction.support.TransactionCallback;
import org.springframework.transaction.support.TransactionCallbackWithoutResult;
import org.springframework.transaction.support.TransactionTemplate;

public abstract class RowInserter<T> extends HibernateDaoSupport implements Closure {
    public enum RowStatus {
        FAILURE, SUCCESS, WARNING_PIN_RESET, WARNING_ALIAS_COLLISION;
    }

    public static final Log LOG = LogFactory.getLog(CsvRowInserter.class);

    private JobContext m_jobContext;

    private PlatformTransactionManager m_transactionManager;

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }

    public void setTransactionManager(PlatformTransactionManager transactionManager) {
        m_transactionManager = transactionManager;
    }

    public final void execute(Object input) {
        T row = (T) input;
        String jobDescription = dataToString(row);
        final Serializable jobId = m_jobContext.schedule("Import data: " + jobDescription);

        TransactionTemplate tt = new TransactionTemplate(m_transactionManager);
        try {
            TransactionCallback callback = new JobTransaction(jobId, row);
            tt.execute(callback);
        } catch (UserException e) {
            // ignore user exceptions - just log them
            m_jobContext.failure(jobId, null, e);
        } catch (RuntimeException e) {
            // log and rethrow other exceptions
            m_jobContext.failure(jobId, null, e);
            throw e;
        }
    }

    protected abstract void insertRow(T input);

    public void afterInsert() {
        getHibernateTemplate().flush();
        getHibernateTemplate().clear();
    }

    /**
     * Should be used to verify data format. If it returns falls insertData is not called. This is
     * just an optimization, that eliminates the cost of tranaction setup for invalid data.
     *
     * @param input - one row of imported data
     * @return if true data can be imported, if false the row will be skipped
     */
    protected RowResult checkRowData(T input) {
        return input == null ? new RowResult(RowStatus.FAILURE) : new RowResult(RowStatus.SUCCESS);
    }

    /**
     * Called before inserting begins, before the first time insertRow is called
     */
    public void beforeInserting() {
        // do nothing
    }

    /**
     * Called after all insertions are doen: after the last one insertRow is called.
     */
    public void afterInserting() {
        // do nothing
    }

    /**
     * Provide user-readable representation of the row
     *
     * @param input - row input
     * @return user-readable representation to be used in logs and UI
     */
    protected abstract String dataToString(T input);

    private final class JobTransaction extends TransactionCallbackWithoutResult {
        private final Serializable m_id;
        private final T m_input;

        private JobTransaction(Serializable id, T input) {
            m_id = id;
            m_input = input;
        }

        protected void doInTransactionWithoutResult(TransactionStatus status_) {
            m_jobContext.start(m_id);
            switch (checkRowData(m_input).getRowStatus()) {
            case SUCCESS:
                insertRow(m_input);
                m_jobContext.success(m_id);
                afterInsert();
                break;
            case FAILURE:
                String errorMessage = "Invalid data format when importing: "
                        + dataToString(m_input);
                String wrongData = checkRowData(m_input).getErrorMessage();
                if (StringUtils.isNotBlank(wrongData)) {
                    errorMessage += " - unsupported value: " + wrongData;
                }
                LOG.warn(errorMessage);
                m_jobContext.failure(m_id, errorMessage, null);
                break;
            case WARNING_PIN_RESET:
                insertRow(m_input);
                String warnMessage = "Unable to import Voicemail PIN: PIN has been reset.";
                LOG.warn(warnMessage);
                m_jobContext.warning(m_id, warnMessage);
                break;
            case WARNING_ALIAS_COLLISION:
                insertRow(m_input);
                warnMessage = "Alias collision - skip alias for: " + dataToString(m_input);
                LOG.warn(warnMessage);
                m_jobContext.warning(m_id, warnMessage);
                afterInsert();
                break;
            default:
                throw new IllegalArgumentException("Need to handle all status cases.");
            }
        }
    }

    public static class RowResult {
        private RowStatus m_rowStatus;

        private String m_errorMessage;

        public RowResult(RowStatus status) {
            m_rowStatus = status;
        }

        public RowResult(RowStatus status, String mess) {
            m_rowStatus = status;
            m_errorMessage = mess;
        }

        public RowStatus getRowStatus() {
            return m_rowStatus;
        }

        public String getErrorMessage() {
            return m_errorMessage;
        }
    }
}
