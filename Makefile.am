SUBDIRS = source

lib_LTLIBRARIES=libwecb.la
libwecb_la_CPPFLAGS= -D_ANSC_LINUX -D_ANSC_USER -D_ANSC_LITTLE_ENDIAN_ -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/include -I$(top_srcdir)/../CcspCommonLibrary/source/debug_api/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/slap/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/http/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/ansc/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/MessageBusHelper/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/PoamIrepFolder -I$(top_srcdir)/../Utopia/source/include -I$(top_srcdir)/source/TR-181/ml -I$(top_srcdir)/source/TR-181/include -I$(top_srcdir)/include/TR-181/ml -I$(top_srcdir)/source/wecb -I$(top_srcdir)/source/Custom -I$(top_srcdir)/./include
libwecb_la_LDFLAGS= -L$(top_builddir)/../CcspCommonLibrary/lib -lccsp_common -L$(top_builddir)/../Utopia/lib -lsysevent -lsyscfg -lulog -lupnp -lxml2 -lcurl -ldbus-1
libwecb_la_SOURCES= \
	source/TR-181/ml/cosa_apis_busutil.c \
	source/TR-181/ml/cosa_apis_util.c \
	source/TR-181/ml/cosa_wifi_dml.c \
	source/TR-181/ml/cosa_wifi_internal.c \
	source/TR-181/ml/plugin_main_apis.c \
	source/TR-181/ml/plugin_main.c \
	source-arm/TR-181/board_sbapi/cosa_wifi_apis.c
