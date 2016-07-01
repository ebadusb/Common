/*
 * Defines for Taos/Trima specific bootrom boot line argument
 */

/* ================================================================================ */

#ifdef OPTIA_BOOTROM

#define INTFTPUSER "david"
#define INTFTPPASS "Aunt_078"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/taos/boot/safety/vxWorks_bengal" \
  " e=90.1.0.1:ffffff00 h=90.1.0.2" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x0"

#define BOOTLOAD_TAG   "TAOS"

#endif /* OPTIA_BOOTROM */
/* ================================================================================ */

#ifdef OPTIA_APC_BOOTROM

#define INTFTPUSER "david"
#define INTFTPPASS "Aunt_078"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/taos/boot/apc/vxWorks_bengal" \
  " e=90.2.0.1:ffffff00 h=90.2.0.2" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x0"

#define BOOTLOAD_TAG   "TAOS"

#endif /* OPTIA_APC_BOOTROM */

/* ================================================================================ */

#ifdef TRIMA_BOOTROM

#ifdef COMMON_KERNEL

#define SFTYFTPUSER "SaF8t9d6er77823afZ"
#define SFTYFTPPASS "Ru33st65890ZbcK3mo"

/* XXX: For Common Kernel; need to revisit this */
#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/config/kernel_init/safety/vxWorks" \
  " e=90.0.0.1:ffffff00 h=90.0.0.2 tn=safety" \
  " u=" SFTYFTPUSER " pw=" SFTYFTPPASS \
  " f=0x0"

#else

#define INTFTPUSER "trima"
#define INTFTPPASS "Dri6SWLe"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/trima/safety/boot/vxWorks" \
  " e=10.0.0.1:ffffff00 h=10.0.0.2 tn=safety" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x0"

#endif

#define BOOTLOAD_TAG   "Trima"

#endif /* TRIMA_BOOTROM */

/* ================================================================================ */
