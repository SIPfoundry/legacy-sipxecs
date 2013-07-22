
# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

# fastdb header have to be installed because
# of compile dependencies in other sipX projects
nobase_include_HEADERS =  \
    sipXecsService/SipXecsService.h \
    sipXecsService/SharedSecret.h \
    sipXecsService/SipNonceDb.h \
    sipXecsService/daemon.h \
    configrpc/ConfigRPC.h \
    sipdb/MongoDB.h \
    sipdb/MongoOpLog.h \
    sipdb/EntityDB.h \
    sipdb/EntityRecord.h \
    sipdb/RegBinding.h \
    sipdb/RegExpireThread.h \
    sipdb/RegDB.h \
    sipdb/SubscribeExpireThread.h \
    sipdb/SubscribeDB.h \
    sipdb/Subscription.h \
    sipdb/ResultSet.h \
    sipdb/GatewayDestDB.h \
    sipdb/GatewayDestRecord.h \
    digitmaps/UrlMapping.h \
    digitmaps/FallbackRulesUrlMapping.h \
    digitmaps/AuthRulesUrlMapping.h \
    digitmaps/EmergencyRulesUrlMapping.h \
    digitmaps/MappingRulesUrlMapping.h \
    digitmaps/Patterns.h \
    filereader/OrbitFileReader.h \
    filereader/RefreshingFileReader.h \
    odbc/OdbcWrapper.h \
    persist/SipPersistentSubscriptionMgr.h
