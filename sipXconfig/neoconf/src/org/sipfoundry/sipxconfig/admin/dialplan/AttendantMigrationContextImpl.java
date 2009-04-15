/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.classic.Session;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class AttendantMigrationContextImpl extends SipxHibernateDaoSupport implements AttendantMigrationContext {
    public static final Log LOG = LogFactory.getLog(AttendantMigrationContextImpl.class);

    private static final String[] SQL = {
        "alter table internal_dialing_rule drop column auto_attendant_id",
        "alter table internal_dialing_rule drop column aa_aliases",
        "alter table auto_attendant drop column extension"
    };

    private static final String RULE_NAME_PREFIX = "Attendant_";
    private AutoAttendantManager m_autoAttendantManager;
    private DialPlanContext m_dialPlanContext;

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void migrateAttendantRules() {
        List data = getHibernateTemplate().findByNamedQuery("attendantsReferencesByInternalRules");
        for (Iterator i = data.iterator(); i.hasNext();) {
            Object[] row = (Object[]) i.next();
            Integer attendantId = (Integer) row[0];
            if (attendantId == null) {
                continue;
            }
            String aliases = (String) row[1];
            String extension = (String) row[2];
            try {
                migrateAttendant(attendantId, aliases, extension);
            } catch (UserException e) {
                // in some cases we will be unable to migrate all rules dues to name or extension
                // conflics
                LOG.warn("cannot migrate rules", e);
            }
        }
        cleanSchema();
    }

    public void setAttendantDefaults() {
        Group defaultGroup = m_autoAttendantManager.getDefaultAutoAttendantGroup();
        for (AutoAttendant aa : m_autoAttendantManager.getAutoAttendants()) {
            if (aa.getGroups().size() == 0) {
                aa.addGroup(defaultGroup);
                m_autoAttendantManager.storeAutoAttendant(aa);
            }
        }
    }

    private void migrateAttendant(Integer attendantId, String aliases, String extension) {
        AutoAttendant autoAttendant = m_autoAttendantManager.getAutoAttendant(attendantId);
        AttendantRule rule = new AttendantRule();
        rule.setName(RULE_NAME_PREFIX + autoAttendant.getName());
        rule.setEnabled(true);
        rule.setExtension(extension);
        if (!StringUtils.isBlank(aliases)) {
            String newAliases = aliases.replaceAll(",\\s*", " ");
            rule.setAttendantAliases(newAliases);
        }
        ScheduledAttendant sa = new ScheduledAttendant();
        sa.setAttendant(autoAttendant);
        sa.setEnabled(true);
        rule.setAfterHoursAttendant(sa);
        m_dialPlanContext.addRule(0, rule);
    }

    private void cleanSchema() {
        try {
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            Connection connection = currentSession.connection();
            Statement statement = connection.createStatement();
            for (int i = 0; i < SQL.length; i++) {
                statement.addBatch(SQL[i]);
            }
            statement.executeBatch();
            statement.close();
        } catch (SQLException e) {
            LOG.warn("cleaning schema", e);
        }
    }
}
