noinst_LTLIBRARIES = libmailboxmgr.la

# do not do versioning for noinst libraries

libmailboxmgr_la_LIBADD = \
	@SIPXCOMMSERVER_LIBS@ \
        @SIPXMEDIAADAPTER_LIBS@ \
	@GIPS_NEQ_OBJS@

libmailboxmgr_la_CXXFLAGS = \
	-I$(top_srcdir)/include/ \
	-I$(top_srcdir)/src \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libmailboxmgr_la_SOURCES = \
	ActiveGreetingHelper.cpp \
	AutoAttendantCGI.cpp \
	CategorizedString.cpp \
	ChangePinCGI.cpp \
	DeleteGreetingCGI.cpp \
	DeleteMailboxCGI.cpp \
	DepositCGI.cpp \
	DialByNameCGI.cpp \
	EditMessageWebCGI.cpp \
	ForwardByDistListCGI.cpp \
	ForwardMessagesCGI.cpp \
	GetAllGreetingsCGI.cpp \
	GetAllSystemPromptsCGI.cpp \
	GetMessageDetailsHelper.cpp \
	GetNavWebCGI.cpp \
	LoginCGI.cpp \
	MailboxManager.cpp \
	ManageDistributionsWebCGI.cpp \
	ManageFoldersWebCGI.cpp \
	ManageNotificationsWebCGI.cpp \
	MessageIDGenerator.cpp \
	MoveMessagesCGI.cpp \
	NotificationHelper.cpp \
	PlayMessagesCGI.cpp \
	RecycleDeletedMsgsCGI.cpp \
	RetrieveCGI.cpp \
	SaveGreetingCGI.cpp \
	SaveMessage.cpp \
	SaveSystemPromptsCGI.cpp \
	SendByDistListCGI.cpp \
	SetActiveGreetingCGI.cpp \
	SetActiveSystemPromptCGI.cpp \
	SpecialAAMenuCGI.cpp \
	SosCGI.cpp \
	StatusServerCGI.cpp \
	TransferToExtnCGI.cpp \
	UpdateMessageStatesCGI.cpp \
	ValidateMailboxCGIHelper.cpp

noinst_HEADERS = \
    ActiveGreetingHelper.h \
    AutoAttendantCGI.h \
    CategorizedString.h \
    CGICommand.h \
    ChangePinCGI.h \
    DeleteGreetingCGI.h \
    DeleteMailboxCGI.h \
    DepositCGI.h \
    DialByNameCGI.h \
    EditMessageWebCGI.h \
    ForwardByDistListCGI.h \
    ForwardMessagesCGI.h \
    GetAllGreetingsCGI.h \
    GetAllSystemPromptsCGI.h \
    GetMessageDetailsHelper.h \
    GetNavWebCGI.h \
    HTMLDefs.h \
    LoginCGI.h \
    MailboxManager.h \
    ManageDistributionsWebCGI.h \
    ManageFoldersWebCGI.h \
    ManageNotificationsWebCGI.h \
    MessageIDGenerator.h \
    MoveMessagesCGI.h \
    NotificationHelper.h \
    PlayMessagesCGI.h \
    RecycleDeletedMsgsCGI.h \
    RetrieveCGI.h \
    SaveGreetingCGI.h \
    SaveMessage.h \
    SaveSystemPromptsCGI.h \
    SendByDistListCGI.h \
    SetActiveGreeting.h \
    SetActiveGreetingCGI.h \
    SetActiveSystemPromptCGI.h \
    SpecialAAMenuCGI.h \
    SosCGI.h \
    StatusServerCGI.h \
    TransferToExtnCGI.h \
    UpdateMessageStatesCGI.h \
    ValidateMailboxCGIHelper.h \
    VXMLDefs.h
