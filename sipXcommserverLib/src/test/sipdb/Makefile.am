INCLUDES = -I$(top_srcdir)/include


AM_CPPFLAGS = \
	-DTEST_DATA_DIR=\"@abs_top_srcdir@/src/test\" 

LDADD = \
	@SIPXUNIT_LIBS@ \
	-lmongoclient \
   ../../libsipXcommserver.la

# Define values used in local database unit tests. This only
# happens if configure was run with --enable-dbtests.


TESTS = \
	RegBindingTest \
	SubscriptionTest \
	EntityRecordTest \
	MongoDBTest \
	RegDBTest \
	EntityDBTest \
	SubscribeDBTest \
	ResultSetTest \
	DbHelperTest \
	RegExpireThreadTest \
	SubscribeExpireThreadTest \
	MongoOpLogTest
	
	


check_PROGRAMS = $(TESTS)
			  

RegDBTest_SOURCES = RegDBTest.cpp
RegBindingTest_SOURCES = RegBindingTest.cpp
SubscriptionTest_SOURCES = SubscriptionTest.cpp
EntityRecordTest_SOURCES = EntityRecordTest.cpp
MongoDBTest_SOURCES = MongoDBTest.cpp
EntityDBTest_SOURCES = EntityDBTest.cpp
SubscribeDBTest_SOURCES = SubscribeDBTest.cpp
ResultSetTest_SOURCES = ResultSetTest.cpp
DbHelperTest_SOURCES = DbHelperTest.cpp
RegExpireThreadTest_SOURCES = RegExpireThreadTest.cpp
SubscribeExpireThreadTest_SOURCES = SubscribeExpireThreadTest.cpp
MongoOpLogTest_SOURCES = MongoOpLogTest.cpp