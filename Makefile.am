##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
SUBDIRS = source

lib_LTLIBRARIES=libwecb.la
libwecb_la_CPPFLAGS= -D_ANSC_LINUX -D_ANSC_USER -D_ANSC_LITTLE_ENDIAN_ -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/custom -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/include -I$(top_srcdir)/../CcspCommonLibrary/source/debug_api/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/include/linux -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include -I$(top_srcdir)/../CcspCommonLibrary/source/cosa/package/slap/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/http/include -I$(top_srcdir)/../CcspCommonLibrary/source/util_api/ansc/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/MessageBusHelper/include -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/common/PoamIrepFolder -I$(top_srcdir)/../Utopia/source/include -I$(top_srcdir)/source/TR-181/ml -I$(top_srcdir)/source/TR-181/include -I$(top_srcdir)/include/TR-181/ml -I$(top_srcdir)/source/wecb -I$(top_srcdir)/source/Custom -I$(top_srcdir)/./include
libwecb_la_LDFLAGS= -lccsp_common -lsysevent -lsyscfg -lulog -lupnp -lxml2 -lcurl -ldbus-1
libwecb_la_SOURCES= \
	source/TR-181/ml/cosa_apis_busutil.c \
	source/TR-181/ml/cosa_apis_util.c \
	source/TR-181/ml/cosa_wifi_dml.c \
	source/TR-181/ml/cosa_wifi_internal.c \
	source/TR-181/ml/plugin_main_apis.c \
	source/TR-181/ml/plugin_main.c \
	source-arm/TR-181/board_sbapi/cosa_wifi_apis.c \
	source/TR-181/ml/cosa_led_dml.c \
	source/TR-181/ml/cosa_led_internal.c \
	source-arm/TR-181/board_sbapi/cosa_led_apis.c 

