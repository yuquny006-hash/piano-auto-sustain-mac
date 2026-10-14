/*************************************************************************
** Mac Prefix for MachO 
**************************************************************************/

#define __CF_USE_FRAMEWORK_INCLUDES__ 1

#if __MWERKS__
#define __DEBUGGING__
#define __NOEXTENSIONS__
#endif

#define PLUGGUI 1

#define MAC 1
#define MACX 1
#define WINDOWS 0
#define SGI 0
#define MOTIF 0
#define BEBOX 0

#define USE_NAMESPACE 0
#define CARBON 1

#define TARGET_API_MAC_CARBON 1
#define TARGET_OS_MAC 1
#define OLDP2C 1
#define OPAQUE_TOOLBOX_STRUCTS 1
#define USENAVSERVICES 1

#define CARBON_EVENTS 1
#define QUARTZ 1
