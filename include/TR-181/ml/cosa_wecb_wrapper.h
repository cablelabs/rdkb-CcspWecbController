/**********************************************************************

    module: cosa_wecb_wrapper.h

        For WECB 

    ---------------------------------------------------------------

    copyright:



    ---------------------------------------------------------------

    description:

        This wrapper file defines all the platform-independent
        functions on logging

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        

    ---------------------------------------------------------------

    revision:

        

**********************************************************************/


#include "ccsp_trace.h"

extern   char*                      g_WecbCompName;


/*
 * Logging wrapper APIs
 */
#define  CcspWecbTraceEmergency(msg)                         \
    CcspTraceEmergency2(g_WecbCompName, msg)

#define  CcspWecbTraceAlert(msg)                             \
    CcspTraceAlert2(g_WecbCompName, msg)

#define  CcspWecbTraceCritical(msg)                          \
    CcspTraceCritical2(g_WecbCompName, msg)

#define  CcspWecbTraceError(msg)                             \
    CcspTraceError2(g_WecbCompName, msg)

#define  CcspWecbTraceWarning(msg)                           \
    CcspTraceWarning2(g_WecbCompName, msg)

#define  CcspWecbTraceNotice(msg)                            \
    CcspTraceNotice2(g_WecbCompName, msg)

#define  CcspWecbTraceDebug(msg)                             \
    CcspTraceInfo2(g_WecbCompName, msg)

#define  CcspWecbTraceInfo(msg)                              \
    CcspTraceInfo2(g_WecbCompName, msg)


