/*
 * Defines for Taos/Trima specific bootrom boot line argument
 */

/* ================================================================================ */

#ifdef OPTIA_BOOTROM

#define INTFTPUSER "david"
#define INTFTPPASS "Aunt_078"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/taos/boot/safety/vxWorks" \
  " e=90.1.0.1:ffffff00 h=90.1.0.2" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x80"

#define BOOTLOAD_TAG   "TAOS"

#endif /* OPTIA_BOOTROM */
/* ================================================================================ */

#ifdef OPTIA_APC_BOOTROM

#define INTFTPUSER "david"
#define INTFTPPASS "Aunt_078"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/taos/boot/apc/vxWorks" \
  " e=90.2.0.1:ffffff00 h=90.2.0.2" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x80"

#define BOOTLOAD_TAG   "TAOS"

#endif /* OPTIA_BOOTROM */

/* ================================================================================ */

#ifdef TRIMA_BOOTROM

#define INTFTPUSER "trima"
#define INTFTPPASS "Dri6SWLe"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control:/trima/safety/boot/vxWorks" \
  " e=10.0.0.1:ffffff00 h=10.0.0.2 tn=safety" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0x80"

#define BOOTLOAD_TAG   "Trima"

#endif /* TRIMA_BOOTROM */

/* ================================================================================ */
