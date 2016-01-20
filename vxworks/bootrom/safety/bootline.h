/*
 * Defines for Taos/Trima specific bootrom boot line argument
 */

/* ================================================================================ */

#ifdef OPTIA_BOOTROM

#define INTFTPUSER "david"
#define INTFTPPASS "Aunt_078"

#define DEFAULT_BOOT_LINE \
  "(0,0)control:/taos/boot/safety/vxWorks" \
  " e=90.1.0.1:ffffff00 h=90.1.0.2" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0"

#endif /* OPTIA_BOOTROM */

/* ================================================================================ */

#ifdef TRIMA_BOOTROM

#define INTFTPUSER "trima"
#define INTFTPPASS "Dri6SWLe"

#define DEFAULT_BOOT_LINE \
  "gei(0,0)control_safety:/trima/safety/boot/vxWorks" \
  " e=10.0.0.1:ffffff00 h=10.0.0.2 tn=safety" \
  " u=" INTFTPUSER " pw=" INTFTPPASS \
  " f=0"

#endif /* TRIMA_BOOTROM */

/* ================================================================================ */
