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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

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
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
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
import org.sipfoundry.sipxconfig.externaltools.ToolbarDownloadContext;
import org.sipfoundry.sipxconfig.login.PrivateUserKeyManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;

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

    @InjectObject(value = "spring:toolbarDownloadContext")
    public abstract ToolbarDownloadContext getToolbarDownloadContext();

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

    public abstract User getLoadedUser();
    public abstract void setLoadedUser(User user);

    public boolean getHasVoicemailPermission() {
        return getLoadedUser().hasPermission(PermissionName.VOICEMAIL);
    }

    public boolean getToolbarInstallerPresent() {
        return getToolbarDownloadContext().isToolbarInstallerPresent();
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
        Collection<Serializable> allSelected = getSelections().getAllSelected();
        for (Serializable id : allSelected) {
            Voicemail vm = getVoicemailSource().getVoicemail(id);
            getMailboxManager().delete(getLoadedUser().getUserName(), vm);
        }
    }

    private String getFolderLabel(String folderId) {
        return getMessages().getMessage("tab." + folderId);
    }

    public IPage edit(Voicemail voicemail) {
        EditVoicemail page = (EditVoicemail) getRequestCycle().getPage(EditVoicemail.PAGE);
        page.setReturnPage(PAGE);
        page.setVoicemail(voicemail);
        return page;
    }

    public ITableColumn getTimestampColumn() {
        return TapestryUtils.createDateColumn("descriptor.timestamp", getMessages(), getExpressionEvaluator(),
                getLocale());
    }

    public PlayVoicemailService.Info getPlayVoicemailInfo() {
        Voicemail voicemail = getVoicemail();
        PlayVoicemailService.Info info = new PlayVoicemailService.Info(voicemail.getFolderId(),
                voicemail.getMessageId(), voicemail.getUserId());
        return info;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getLoadedUser() == null) {
            setLoadedUser(getUser());
        }
        // this needs to be first as it may alter data gathered from subsequent steps in this
        // method
        MailboxOperation operation = getMailboxOperation();
        if (operation != null) {
            String expectedUserId = getLoadedUser().getUserName();
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

        String userId = getLoadedUser().getUserName();

        MailboxManager mgr = getMailboxManager();
        List<String> folderIds = mgr.getFolderIds();
        if (getFolderIds() == null) {
            setFolderIds(folderIds);
        }

        String folderId = getFolderId();
        if (folderId == null || !folderIds.contains(folderId)) {
            setFolderId(folderIds.get(0));
        }

        VoicemailSource source = null;
        List<Voicemail> voicemails = Collections.EMPTY_LIST;
        Map<Serializable, Voicemail> voicemailMap = new LinkedHashMap<Serializable, Voicemail>();
        try {
            voicemails = mgr.getVoicemail(userId, getFolderId());
            for (Voicemail voicemail : voicemails) {
                voicemailMap.put(VoicemailSource.getVoicemailId(voicemail), voicemail);
            }
        } catch (UserException e) {
            getValidator().record(e, getMessages());
        }
        source = new VoicemailSource(voicemailMap);
        setVoicemailSource(source);
        setRowInfo(new VoicemailRowInfo());
        setConverter(new VoicemailSqueezer(source));
    }

    /**
     * Lazily get voicemails to avoid tapestry bug that attempts to use table collection before
     * pageBeginRender is called when navigating table pager
     */
    public Collection<Voicemail> getVoicemails() {
        return getVoicemailSource().getVoicemails();
    }

    public static class VoicemailRowInfo implements RowInfo {
        public Object getSelectId(Object row) {
            return VoicemailSource.getVoicemailId((Voicemail) row);
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
    public void call(String from) {
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getLoadedUser().getAddrSpec(domain);
        String destAddrSpec = SipUri.format(from, domain, false);
        if (destAddrSpec != null) {
            String displayName = "ClickToCall";
            getSipService().sendRefer(getUser(), userAddrSpec, displayName, destAddrSpec);
        } else {
            LOG.error("Failed to get URI to call: " + from);
        }
    }

    public String getFeedLink() {
        String key = getPrivateUserKeyManager().getPrivateKeyForUser(getLoadedUser());
        return String.format("/sipxconfig/rest/private/%s/feed/voicemail/inbox", key);
    }

    public String getVoicemailLink() {
        Voicemail voicemail = getVoicemail();
        PlayVoicemailService.Info info = new PlayVoicemailService.Info(voicemail.getFolderId(),
                voicemail.getMessageId(), voicemail.getUserId());
        ILink link = getPlayVoicemailService().getLink(false, info);
        return link.getURL();

    }

    public String getDownloadToolbarLabel() {
        return getMessages().format("prompt.installtoolbar", getMessages().getMessage("product.name.short"));
    }
}
