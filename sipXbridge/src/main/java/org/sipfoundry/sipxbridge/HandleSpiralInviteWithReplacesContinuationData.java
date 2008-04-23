package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;
import javax.sip.ServerTransaction;

class HandleSpiralInviteWithReplacesContinuationData {
    RequestEvent requestEvent;
    Dialog replacedDialog;
    ServerTransaction serverTransaction;
    String toDomain;
    boolean isPhone;

    public HandleSpiralInviteWithReplacesContinuationData(
            RequestEvent requestEvent, Dialog replacedDialog,
            ServerTransaction serverTransaction, String toDomain,
            boolean isphone) {
        this.requestEvent = requestEvent;
        this.replacedDialog = replacedDialog;
        this.serverTransaction = serverTransaction;
        this.toDomain = toDomain;
        this.isPhone = isphone;

    }
}
