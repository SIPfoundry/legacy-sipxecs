/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import java.util.LinkedHashSet;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.split;
import static org.apache.commons.lang.StringUtils.trim;

public class SipxAccCodeService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxAccCodeService";
    public static final String LOG_SETTING = "acccode/log.level";
    public static final String AUTH_CODE_PREFIX = "authcode/SIP_AUTH_CODE_PREFIX";
    public static final String AUTH_CODE_ALIASES = "authcode/SIP_AUTH_CODE_ALIASES";
    private static final Log LOG = LogFactory.getLog(SipxAccCodeService.class);
    private static final String EXTENSION = "extension";

    private AliasManager m_aliasManager;

    private SipxReplicationContext m_replicationContext;

    private String m_promptsDir;
    private String m_docDir;
    private String m_authcodeprefix;

    private String m_aliases;

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setPromptsDir(String promptsDirectory) {
        m_promptsDir = promptsDirectory;
    }

    public String getPromptsDir() {
        return m_promptsDir;
    }

    @Required
    public void setDocDir(String docDirectory) {
        m_docDir = docDirectory;
    }

    public String getDocDir() {
        return m_docDir;
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }


/**
 *      * Validates the data in this service and throws a UserException if there is a problem
 **/
    @Override
    public void validate() {
        String extension = this.getSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX);
        if (!m_aliasManager.canObjectUseAlias(this, extension)) {
            LOG.info("SipxAccCodeService::validate() canObjectUseAlias() failed.  extension:" + extension);
            throw new NameInUseException(EXTENSION, extension);
        }

        String aliases = this.getSettingValue(SipxAccCodeService.AUTH_CODE_ALIASES);
        for (String alias : getAliasesSet(aliases)) {
            if (!m_aliasManager.canObjectUseAlias(this, alias)) {
                LOG.info("SipxAccCodeService::apply() failed alias check.  alias:" + alias);
                throw new NameInUseException("alias", alias);
            }
        }
    }
    /** get the aliases from a space-delimited string */
    public Set<String> getAliasesSet(String aliasesString) {
        LOG.info(String.format("SipxAccCodeService::getAliasesString(): input:%s:", aliasesString));

        Set<String> aliasesSet = new LinkedHashSet<String>(0);

        if (aliasesString != null) {
            String[] aliases = split(aliasesString);
            for (String alias : aliases) {
                aliasesSet.add(trim(alias));
            }
        }
        LOG.info(String.format("SipxAccCodeService::getAliasesString(): retun set :%s:",  aliasesSet));
        return aliasesSet;
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    @Override
    public void onConfigChange() {
        m_authcodeprefix = getSettingValue(AUTH_CODE_PREFIX);
        m_aliases = getSettingValue(AUTH_CODE_ALIASES);
        LOG.info(String.format("SipxAccCodeService::onConfigChange(): set prefix", m_authcodeprefix));
        LOG.info(String.format("SipxAccCodeService::onConfigChange(): set aliases", m_aliases));
        LOG.info(String.format("SipxAccCodeService::onConfigChange(): replicate ", DataSet.ALIAS));
        m_replicationContext.generate(DataSet.ALIAS);

    }

    public String getAuthCodePrefix() {
        String prefix = getSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX);
        return prefix;
    }

    public void setAuthCodePrefix(String authcodeprefix) {
        m_authcodeprefix = authcodeprefix;
        setSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX, authcodeprefix);
    }

    public String getAuthCodeAliases() {
        String aliases = getSettingValue(SipxAccCodeService.AUTH_CODE_PREFIX);
        return aliases;
    }

    public void setAuthCodeAliases(String aliases) {
        m_aliases = aliases;
        setSettingValue(SipxAccCodeService.AUTH_CODE_ALIASES, aliases);
    }

    /** get the aliases from a space-delimited string */
    public Set<String> getAliasesAsSet() {

        String aliasesString = this.getSettingValue(SipxAccCodeService.AUTH_CODE_ALIASES);
        LOG.info(String.format("SipxAccCodeService::getAliasesAsSet(): %s:", aliasesString));

        Set<String> aliasesSet = new LinkedHashSet<String>(0);

        if (aliasesString != null) {
            String[] aliases = split(aliasesString);
            for (String alias : aliases) {
                aliasesSet.add(trim(alias));
            }
        }
        LOG.info(String.format("SipxAccCodeService::getAliasesAsSet(): return set :%s:",  aliasesSet));
        return aliasesSet;
    }

}
