/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.vm;

import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IExternalPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.RowInfo;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailSource;

public abstract class ManageVoicemail extends UserBasePage implements IExternalPage {

    public static final String PAGE = "vm/ManageVoicemail";

    private static final Log LOG = LogFactory.getLog(ManageVoicemail.class);

    @Asset("/images/play.png")
    public abstract IAsset getPlayVoicemailAsset();

    @Asset("/images/email.png")
    public abstract IAsset getNewVoicemailIcon();

    @Asset("/images/email_open.png")
    public abstract IAsset getHeardVoicemailIcon();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InjectObject("spring:sip")
    public abstract SipService getSipService();

    @InjectObject("spring:privateUserKeyManager")
    public abstract PrivateUserKeyManager getPrivateUserKeyManager();

    public abstract VoicemailSource getVoicemailSource();

    public abstract void setVoicemailSource(VoicemailSource source);

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    public abstract Voicemail getVoicemail();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    public abstract void setConverter(IPrimaryKeyConverter converter);

    public abstract VoicemailRowInfo getRowInfo();

    public abstract void setRowInfo(VoicemailRowInfo rowInfo);

    public abstract List<String> getFolderIds();

    public abstract void setFolderIds(List<String> folderIds);

    @Persist(value = "session")
    public abstract String getFolderId();

    public abstract void setFolderId(String folderId);

    @InjectObject(value = "engine-service:" + PlayVoicemailService.SERVICE_NAME)
    public abstract IEngineService getPlayVoicemailService();

    public abstract MailboxOperation getMailboxOperation();

    public abstract void setMailboxOperation(MailboxOperation operation);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public boolean getHasVoicemailPermission() {
        return getUser().hasPermission(PermissionName.VOICEMAIL);
    }

    public IAsset getVoicemailIcon() {
        Voicemail voicemail = getVoicemail();
        return voicemail.isHeard() ? getHeardVoicemailIcon() : getNewVoicemailIcon();
    }

    public Object getRowClass() {
        return new HeardEvenOdd(this);
    }

    public class HeardEvenOdd extends EvenOdd {
        private final ManageVoicemail m_page;

        public HeardEvenOdd(ManageVoicemail page) {
            m_page = page;
        }

        @Override
        public String getNext() {
            String style = super.getNext();
            if (!m_page.getVoicemail().isHeard()) {
                style = style + "-unheard";
            }
            return style;
        }
    }

    public void activateExternalPage(Object[] parameters, IRequestCycle cycle) {
        String sparam = parameters[0].toString();
        MailboxOperation operation = MailboxOperation.createMailboxOperationFromServletPath(sparam);
        setFolderId(operation.getFolderId());
        setMailboxOperation(operation);
    }

    public IPropertySelectionModel getActionModel() {
        Collection actions = new ArrayList();
        actions.add(new OptGroup(getMessages().getMessage("label.moveTo")));
        for (String folderId : getFolderIds()) {
            if (!folderId.equals(getFolderId())) {
                OptionAdapter action = new MoveVoicemailAction(getMailboxManager(), getVoicemailSource(),
                        getFolderLabel(folderId), folderId);
                actions.add(action);
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);
        return model;
    }

    public void delete() {
        Mailbox mbox = getMailboxManager().getMailbox(getUser().getUserName());
        Collection<Serializable> allSelected = getSelections().getAllSelected();
        for (Serializable id : allSelected) {
            Voicemail vm = getVoicemailSource().getVoicemail(id);
            getMailboxManager().delete(mbox, vm);
        }
    }

    public String getFolderLabel() {
        return getFolderLabel(getFolderId());
    }

    String getFolderLabel(String folderId) {
        return getMessages().getMessage("tab." + folderId);
    }

    public IPage edit(String voicemailId) {
        EditVoicemail page = (EditVoicemail) getRequestCycle().getPage(EditVoicemail.PAGE);
        page.setReturnPage(PAGE);
        page.setVoicemailId(voicemailId);
        return page;
    }

    public ITableColumn getTimestampColumn() {
        return TapestryUtils.createDateColumn("descriptor.timestamp", getMessages(), getExpressionEvaluator(),
                getLocale());
    }

    public PlayVoicemailService.Info getPlayVoicemailInfo() {
        Voicemail voicemail = getVoicemail();
        PlayVoicemailService.Info info = new PlayVoicemailService.Info(voicemail.getFolderId(), voicemail
                .getMessageId());
        return info;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        // this needs to be first as it may alter data gathered from subsequent steps in this
        // method
        MailboxOperation operation = getMailboxOperation();
        if (operation != null) {
            String expectedUserId = getUser().getUserName();
            if (!expectedUserId.equals(operation.getUserId())) {
                String msg = String.format("Unauthorized access attempted to mailbox for user %s from user %s",
                        operation.getUserId(), expectedUserId);
                LOG.warn(msg);
                throw new PageRedirectException(msg);
            }
            setMailboxOperation(null);
            operation.operate(this);
        }

        SelectMap selections = getSelections();
        if (selections == null) {
            setSelections(new SelectMap());
        }

        String userId = getUser().getUserName();

        MailboxManager mgr = getMailboxManager();
        Mailbox mbox = mgr.getMailbox(userId);
        List<String> folderIds = getFolderIds();
        if (getFolderIds() == null) {
            folderIds = mbox.getFolderIds();
            setFolderIds(folderIds);
        }

        String folderId = getFolderId();
        if (folderId == null || !folderIds.contains(folderId)) {
            setFolderId(folderIds.get(0));
        }

        VoicemailSource source = getVoicemailSource();
        if (source == null) {
            source = new VoicemailSource(new File(getMailboxManager().getMailstoreDirectory()));
            setVoicemailSource(source);
            setRowInfo(new VoicemailRowInfo(source));
            setConverter(new VoicemailSqueezer(source));
        }
    }

    /**
     * Lazily get voicemails to avoid tapestry bug that attempts to use table collection before
     * pageBeginRender is called when navigating table pager
     */
    public List<Voicemail> getVoicemails() {
        MailboxManager mgr = getMailboxManager();
        String userId = getUser().getUserName();
        Mailbox mbox = mgr.getMailbox(userId);
        try {
            return mgr.getVoicemail(mbox, getFolderId());
        } catch (UserException e) {
            getValidator().record(new ValidatorException(e.getMessage()));
            return Collections.emptyList();
        }
    }

    public static class VoicemailRowInfo implements RowInfo {
        private final VoicemailSource m_source;

        VoicemailRowInfo(VoicemailSource source) {
            m_source = source;
        }

        public Object getSelectId(Object row) {
            return m_source.getVoicemailId((Voicemail) row);
        }

        public boolean isSelectable(Object row) {
            return true;
        }
    }

    /**
     * Implements click to call link
     *
     * @param number number to call - refer is sent to current user
     */
    public void call(String fromUri) {
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getUser().getAddrSpec(domain);
        String destAddrSpec = SipUri.extractAddressSpec(fromUri);
        if (destAddrSpec != null) {
            String displayName = "ClickToCall";
            getSipService().sendRefer(getUser(), userAddrSpec, displayName, destAddrSpec);
        } else {
            LOG.error("Failed to get URI to call: " + fromUri);
        }
    }

    public String getFeedLink() {
        String key = getPrivateUserKeyManager().getPrivateKeyForUser(getUser());
        return String.format("/sipxconfig/rest/private/%s/feed/voicemail/inbox", key);
    }

    public String getVoicemailLink() {
        Voicemail voicemail = getVoicemail();
        String voicemailLink = String.format("/sipxconfig/rest/my/voicemail/%s/%s", voicemail.getFolderId(),
                voicemail.getMessageId());
        /*
         * <HACK> -----------
         * A .wav suffix is added at the end of the url to correctly associate the file
         * as a .wav file, when downloading it through the <audio/> controller in FF 3.5
         *
         */

        voicemailLink = voicemailLink.concat(".wav");

        /*
         * </HACK>
         */
        return voicemailLink;
    }
}
