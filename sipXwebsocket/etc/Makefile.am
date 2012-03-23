include $(top_srcdir)/config/utility.am

cfinputsdir = @SIPX_CFINPUTS@/plugin.d
dist_cfinputs_DATA = \
	sipxwebsocket.cf

webinfdir = @SIPX_JAVADIR@/sipXwebsocket/WEB-INF
dist_webinf_DATA = \
	context/WEB-INF/web.xml

beansdir = @SIPX_JAVADIR@/sipXwebsocket
dist_beans_DATA = $(shell cd $(srcdir); find context -name '*.beans.xml')