/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 * Copyright 1984-1995 Wind River Systems, Inc.
 *
 * vxWorks serial port driver with support for Ampro LB-486
 *
 *	This driver is based on the i8250sio.c vxWorks driver, with the
 * following modifications:
 *		- support for handling multiple serial ports on a single interrupt
 *		- support for full range of baud rates
 *
 * $Header: K:/BCT_Development/vxWorks/Common/os/rcs/serial_drv.c 1.1 2002/06/07 21:43:16 jl11312 Exp jl11312 $
 * $Log: serial_drv.c $
 * Revision 1.1  2002/06/07 21:43:16  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <errnoLib.h>
#include <iv.h>
#include <intLib.h>
#include <semLib.h>
#include <sioLib.h>
#include <stdlib.h>

#include "serial_drv.h"

/* channel data structure  */

typedef struct stI8250_CHAN   		 /* I8250_CHAN */
    {
    SIO_DRV_FUNCS * pDrvFuncs;       /* driver functions - must be first entry in struct */

    STATUS      (*getTxChar) ();     /* pointer to xmitr function */
    STATUS      (*putRcvChar) ();    /* pointer tp rcvr function */
    void *      getTxArg;
    void *      putRcvArg;

    UINT16  int_vec;                 /* interrupt vector number */
    UINT16  channelMode;             /* SIO_MODE_[INT | POLL] */
    UCHAR   (*inByte) (int);         /* routine to read a byte from register */
    void    (*outByte)(int,char);    /* routine to write a byte to register */

    ULONG   lcr;                     /* UART line control register */
    ULONG   lst;                     /* UART line status register */
    ULONG   mdc;                     /* UART modem control register */
    ULONG   msr;                     /* UART modem status register */
    ULONG   ier;                     /* UART interrupt enable register */
    ULONG   iid;                     /* UART interrupt ID register */
    ULONG   brdl;                    /* UART baud rate register */
    ULONG   brdh;                    /* UART baud rate register */
    ULONG   data;                    /* UART data register */
    ULONG   options;		     			 /* UART hardware options */

	 unsigned int baseAddress;			 /* base I/O address for serial port controller chip */
	 struct stI8250_CHAN * intChain;	 /* linked list of serial ports on this same interrupt */
	 struct stI8250_CHAN * portChain; /* linked list of all serial ports */

	 unsigned char savedIER;			 /* saved interrupt enable register value */
    } I8250_CHAN;

/* register definitions */

#define UART_THR        0x00	/* Transmitter holding reg. */
#define UART_RDR        0x00	/* Receiver data reg.       */
#define UART_BRDL       0x00	/* Baud rate divisor (LSB)  */
#define UART_BRDH       0x01	/* Baud rate divisor (MSB)  */
#define UART_IER        0x01	/* Interrupt enable reg.    */
#define UART_IID        0x02	/* Interrupt ID reg.        */
#define UART_LCR        0x03	/* Line control reg.        */
#define UART_MDC        0x04	/* Modem control reg.       */
#define UART_LST        0x05	/* Line status reg.         */
#define UART_MSR        0x06	/* Modem status reg.        */

/* equates for interrupt enable register */

#define I8250_IER_RXRDY	0x01	/* receiver data ready */
#define I8250_IER_TBE	0x02	/* transmit bit enable */
#define I8250_IER_LST	0x04	/* line status interrupts */
#define I8250_IER_MSI	0x08	/* modem status interrupts */

/* equates for interrupt identification register */

#define I8250_IIR_IP	0x01	/* interrupt pending bit */
#define I8250_IIR_MASK	0x07	/* interrupt id bits mask */
#define I8250_IIR_MSTAT  0x00	/* modem status interrupt */
#define I8250_IIR_THRE	0X02	/* transmit holding register empty */
#define I8250_IIR_RBRF	0x04	/* receiver buffer register full */
#define I8250_IIR_SEOB	0x06	/* serialization error or break */

/* equates for line control register */

#define I8250_LCR_CS5	0x00	/* 5 bits data size */
#define I8250_LCR_CS6	0x01	/* 6 bits data size */
#define I8250_LCR_CS7	0x02	/* 7 bits data size */
#define I8250_LCR_CS8	0x03	/* 8 bits data size */
#define I8250_LCR_2_STB  0x04	/* 2 stop bits */
#define I8250_LCR_1_STB	0x00	/* 1 stop bit */
#define I8250_LCR_PEN	0x08	/* parity enable */
#define I8250_LCR_PDIS   0x00	/* parity disable */
#define I8250_LCR_EPS	0x10	/* even parity slect */
#define I8250_LCR_SP	0x20	/* stick parity select */
#define I8250_LCR_SBRK	0x40	/* break control bit */
#define I8250_LCR_DLAB	0x80	/* divisor latch access enable */

/* equates for the modem control register */

#define I8250_MCR_DTR	0x01	/* dtr output */
#define I8250_MCR_RTS	0x02	/* rts output */
#define I8250_MCR_OUT1	0x04	/* output #1 */
#define I8250_MCR_OUT2	0x08	/* output #2 */
#define I8250_MCR_LOOP	0x10	/* loop back */
 
/* equates for line status register */

#define I8250_LSR_RXRDY	0x01	/* receiver data available */
#define I8250_LSR_OE	0x02	/* overrun error */
#define I8250_LSR_PE	0x04	/* parity error */
#define I8250_LSR_FE	0x08	/* framing error */
#define I8250_LSR_BI	0x10	/* break interrupt */
#define I8250_LSR_THRE	0x20	/* transmit holding register empty */
#define I8250_LSR_TEMT	0x40	/* transmitter empty */

/* equates for modem status register */

#define I8250_MSR_DCTS	0x01	/* cts change */
#define I8250_MSR_DDSR	0x02	/* dsr change */
#define I8250_MSR_DRI	0x04	/* ring change */
#define I8250_MSR_DDCD	0x08	/* data carrier change */
#define I8250_MSR_CTS	0x10	/* complement of cts */
#define I8250_MSR_DSR	0x20	/* complement of dsr */
#define I8250_MSR_RI	0x40	/* complement of ring signal */
#define I8250_MSR_DCD	0x80	/* complement of dcd */

/*
 * This value is used to specify the maximum number of times i8250Int()
 * routine will read the 8250 Interrupt Identification Register before
 * returning.  This value should optimally be derived from the BRDR.
 */
#define I8250_IIR_READ_MAX          10

#ifndef I8250_DEFAULT_BAUD
#   define  I8250_DEFAULT_BAUD      9600
#endif

#ifndef SIO_HUP
#   define SIO_OPEN     0x100A
#   define SIO_HUP      0x100B
#endif

static SIO_DRV_FUNCS i8250SioDrvFuncs;
static I8250_CHAN	*	pFirstChan = NULL;
static SEM_ID			chanAccess = NULL;
static int				portNumber = 0;

/*
 * Local functions
 */
static STATUS i8250BaudSet(I8250_CHAN *, UINT);
static int i8250CallbackInstall(SIO_CHAN * pSioChan, int callbackType, STATUS (*callback)(), void * callbackArg);
static void i8250HandleInterrupt(I8250_CHAN  * pChan);
static void i8250HrdInit(I8250_CHAN * pChan);
static STATUS i8250Hup(I8250_CHAN * pChan);
static void i8250InitChannel (I8250_CHAN *);
static int i8250Ioctl(I8250_CHAN *, int, int);
static STATUS i8250ModeSet(I8250_CHAN *, UINT);
static STATUS i8250Open(I8250_CHAN * pChan);
static STATUS i8250OptsSet(I8250_CHAN *, UINT);
static int i8250Startup(I8250_CHAN *pChan);
static int i8250PRxChar(I8250_CHAN *pChan, char *ch);
static int i8250PTxChar(I8250_CHAN *pChan, char ch);

/******************************************************************************
*
* i8250CallbackInstall - install ISR callbacks to get put chars.
*
* This routine installs the callback functions for the driver
*
* RETURNS: OK on success or ENOSYS on unsupported callback type.
*/ 
static int i8250CallbackInstall
    (
    SIO_CHAN *  pSioChan,
    int         callbackType,
    STATUS      (*callback)(),
    void *      callbackArg
    )
    {
    I8250_CHAN * pChan = (I8250_CHAN *)pSioChan;


    switch (callbackType)
        {
        case SIO_CALLBACK_GET_TX_CHAR:
            pChan->getTxChar    = callback;
            pChan->getTxArg     = callbackArg;
            return (OK);
        case SIO_CALLBACK_PUT_RCV_CHAR:
            pChan->putRcvChar   = callback;
            pChan->putRcvArg    = callbackArg;
            return (OK);
        default:
            return (ENOSYS);
        }
    }

/******************************************************************************
*
* i8250HrdInit - initialize the chip
*
* This routine is called to reset the chip in a quiescent state.
* RETURNS: N/A
*/
static void i8250HrdInit
    (
    I8250_CHAN *  pChan         /* pointer to device */
    )
    {
    if (i8250SioDrvFuncs.ioctl == NULL)
        {
        i8250SioDrvFuncs.ioctl          = (int (*)())i8250Ioctl;
        i8250SioDrvFuncs.txStartup      = (int (*)())i8250Startup;
        i8250SioDrvFuncs.callbackInstall = i8250CallbackInstall;
        i8250SioDrvFuncs.pollInput      = (int (*)())i8250PRxChar;
        i8250SioDrvFuncs.pollOutput     = (int (*)(SIO_CHAN *,char))i8250PTxChar;
        }


    pChan->pDrvFuncs = &i8250SioDrvFuncs;


    i8250InitChannel(pChan);    /* reset the channel */
    }

/*******************************************************************************
*
* i8250InitChannel  - initialize a single channel
*/
static void i8250InitChannel
    (
    I8250_CHAN *  pChan         /* pointer to device */
    )
    {
    int oldLevel;


    oldLevel = intLock ();


    /* set channel baud rate */


    (void) i8250BaudSet(pChan, I8250_DEFAULT_BAUD);


    /* 8 data bits, 1 stop bit, no parity */


    (*pChan->outByte) (pChan->lcr, (I8250_LCR_CS8 | I8250_LCR_1_STB));


    (*pChan->outByte) (pChan->mdc, (I8250_MCR_RTS | I8250_MCR_DTR | \
                                        I8250_MCR_OUT2));



    (*pChan->inByte) (pChan->data);     /* clear the port */


    /* disable interrupts  */


    (*pChan->outByte) (pChan->ier, 0x0);


    pChan->options = (CLOCAL | CREAD | CS8);


    intUnlock (oldLevel);
    }

/*******************************************************************************
*
* i8250Hup - hang up the modem control lines 
*
* Resets the RTS and DTR signals.
*
* RETURNS: OK
*/
static STATUS i8250Hup
    (
    I8250_CHAN * pChan          /* pointer to channel */
    )
    {
    FAST int     oldlevel;      /* current interrupt level mask */


    oldlevel = intLock ();


    (*pChan->outByte) (pChan->mdc, I8250_MCR_OUT2);


    intUnlock (oldlevel);


    return (OK);


    }    

/*******************************************************************************
*
* i8250Open - Set the modem control lines 
*
* Set the modem control lines(RTS, DTR) TRUE if not already set.  
* It also clears the receiver. 
*
* RETURNS: OK
*/
static STATUS i8250Open
    (
    I8250_CHAN * pChan          /* pointer to channel */
    )
    {
    FAST int     oldlevel;      /* current interrupt level mask */
    char mask;


    /* read modem control register */


    mask = ((*pChan->inByte) (pChan->mdc)) & (I8250_MCR_RTS | I8250_MCR_DTR);


    if (mask != (I8250_MCR_RTS | I8250_MCR_DTR)) 
        {
        /* RTS and DTR not set yet */


        oldlevel = intLock ();


        /* set RTS and DTR TRUE */


        (*pChan->outByte) (pChan->mdc, (I8250_MCR_RTS | I8250_MCR_DTR | \
                                        I8250_MCR_OUT2));


        intUnlock (oldlevel);
        }


    return (OK);
    }

/******************************************************************************
*
* i8250BaudSet - change baud rate for channel
*
* This routine sets the baud rate for the UART. The interrupts are disabled
* during chip access.
*
* RETURNS: OK to indicate success, otherwise ERROR is returned
*/
static STATUS  i8250BaudSet
    (
    I8250_CHAN *   pChan,       /* pointer to channel */
    UINT           baud         /* requested baud rate */
    )
    {
    int         oldlevel;
    STATUS      status;
    FAST        int     ix;
    UINT8       lcr;
    int			 divisor;


    /* disable interrupts during chip access */
    oldlevel = intLock ();
	 status = ERROR;

	 if ( baud >= 50 && baud < 115200 )
	 {
		divisor = 115200/baud;

      lcr = (*pChan->inByte) (pChan->lcr);   
      (*pChan->outByte) (pChan->lcr, (char)(I8250_LCR_DLAB | lcr));
      (*pChan->outByte) (pChan->brdh, ((divisor&0xff00) >> 8));
      (*pChan->outByte) (pChan->brdl, divisor&0xff);
      (*pChan->outByte) (pChan->lcr, lcr);
      status = OK;
	 }

    intUnlock(oldlevel);
    return (status);
    }

/*******************************************************************************
*
* i8250ModeSet - change channel mode setting
*
* This driver supports both polled and interrupt modes and is capable of
* switching between modes dynamically. 
*
* If interrupt mode is desired this routine enables the channels receiver and 
* transmitter interrupts. If the modem control option is TRUE, the Tx interrupt
* is disabled if the CTS signal is FALSE. It is enabled otherwise. 
*
* If polled mode is desired the device interrupts are disabled. 
*
* RETURNS:
* Returns a status of OK if the mode was set else ERROR.
*/
static STATUS i8250ModeSet
    (
    I8250_CHAN * pChan,         /* pointer to channel */
    UINT        newMode         /* mode requested */
    )
    {
    FAST int     oldlevel;      /* current interrupt level mask */
    char ier, mask;


    if ((newMode != SIO_MODE_POLL) && (newMode != SIO_MODE_INT))
        return (ERROR);


    oldlevel = intLock();
    (*pChan->outByte) (pChan->ier, 0);

    if (newMode == SIO_MODE_POLL)
        ier = 0x00;
    else  
        {
        if (pChan->options & CLOCAL) 
            ier = I8250_IER_RXRDY;
        else  
            {
            mask = ((*pChan->inByte) (pChan->msr)) & I8250_MSR_CTS;


            /* if the CTS is asserted enable Tx interrupt */


            if (mask & I8250_MSR_CTS)
					 {
                ier = (I8250_IER_TBE | I8250_IER_MSI);
					 }
            else
					 {
                ier = (I8250_IER_MSI);
					 }
            }   
        }


    (*pChan->outByte) (pChan->ier, ier);
            
    pChan->channelMode = newMode;  
          
    intUnlock(oldlevel);


    return (OK);
    }

/*******************************************************************************
*
* i8250OptsSet - set the serial options
*
* Set the channel operating mode to that specified.  All sioLib options
* are supported: CLOCAL, HUPCL, CREAD, CSIZE, PARENB, and PARODD.
*
* RETURNS:
* Returns OK to indicate success, otherwise ERROR is returned
*/
static STATUS i8250OptsSet
    (
    I8250_CHAN * pChan,         /* pointer to channel */
    UINT options                /* new hardware options */
    )
    {
    FAST int     oldlevel;              /* current interrupt level mask */
    char lcr = 0; 
    char mcr = I8250_MCR_OUT2;
    char ier;
    char mask;


    ier = (*pChan->inByte) (pChan->ier);


    if (pChan == NULL || options & 0xffffff00)
        return ERROR;


    switch (options & CSIZE)
        {
        case CS5:
            lcr = I8250_LCR_CS5; break;
        case CS6:
            lcr = I8250_LCR_CS6; break;
        case CS7:
            lcr = I8250_LCR_CS7; break;
        default:
        case CS8:
            lcr = I8250_LCR_CS8; break;
        }


    if (options & STOPB)
        lcr |= I8250_LCR_2_STB;
    else
        lcr |= I8250_LCR_1_STB;
    
    switch (options & (PARENB | PARODD))
        {
        case PARENB|PARODD:
            lcr |= I8250_LCR_PEN; break;
        case PARENB:
            lcr |= (I8250_LCR_PEN | I8250_LCR_EPS); break;
        default:
        case 0:
            lcr |= I8250_LCR_PDIS; break;
        }


    (*pChan->outByte) (pChan->ier, 0x00);


    if (!(options & CLOCAL))
        {
        /* !clocal enables hardware flow control(DTR/DSR) */


        mcr |= (I8250_MCR_DTR | I8250_MCR_RTS);
        ier |= I8250_IER_MSI;    /* enable modem status interrupt */


        mask = ((*pChan->inByte) (pChan->msr)) & I8250_MSR_CTS;


        /* if the CTS is asserted enable Tx interrupt */


        if (mask & I8250_MSR_CTS)
				{
            ier |= I8250_IER_TBE;
			   }
        else
				{
            ier &= (~I8250_IER_TBE);
            }


        }
    else 
        ier &= ~I8250_IER_MSI; /* disable modem status interrupt */ 


    oldlevel = intLock ();


    (*pChan->outByte) (pChan->lcr, lcr);
    (*pChan->outByte) (pChan->mdc, mcr);


    /* now clear the port */

    (*pChan->inByte) (pChan->data);


    if (options & CREAD)  
        ier |= I8250_IER_RXRDY;


    if (pChan->channelMode == SIO_MODE_INT)
        {
        (*pChan->outByte) (pChan->ier, ier);
        }


    intUnlock (oldlevel);


    pChan->options = options;


    return OK;
    }

/*******************************************************************************
*
* i8250Ioctl - special device control
*
* Includes commands to get/set baud rate, mode(INT, POLL), hardware options(
* parity, number of data bits), and modem control(RTS/CTS and DTR/DSR).
* The ioctl commands SIO_HUP and SIO_OPEN are implemented to provide the 
* HUPCL( hang up on last close) function.
*
* RETURNS: OK on success, EIO on device error, ENOSYS on unsupported
*          request.
*/
static int i8250Ioctl
    (
    I8250_CHAN *  pChan,        /* device to control */
    int request,                /* request code */
    int arg                     /* some argument */
    )
    {
    int ix;
    int divisor;
    int oldlevel;
    UINT8 lcr;
    int status = OK;

    switch (request)
        {
        case SIO_BAUD_SET:
            status = (i8250BaudSet(pChan, arg) == OK) ? OK : EIO;
            break;


        case SIO_BAUD_GET:
            
            oldlevel = intLock();


            status = EIO;

            lcr = (*pChan->inByte) (pChan->lcr);
            (*pChan->outByte) (pChan->lcr, (char)(I8250_LCR_DLAB | lcr));
            divisor = 256 * (*pChan->inByte)(pChan->brdh);
            divisor += (*pChan->inByte)(pChan->brdl);
            (*pChan->outByte) (pChan->lcr, lcr);

				if ( divisor > 0 )
				{
					*(int *)arg = 115200/divisor;
				}
				else
				{
					*(int *)arg = 0;
				}

            intUnlock(oldlevel);
            break;



        case SIO_MODE_SET:
            status = (i8250ModeSet (pChan, arg) == OK) ? OK : EIO;
            break;


        case SIO_MODE_GET:
            *(int *)arg = pChan->channelMode;
            return (OK);


        case SIO_AVAIL_MODES_GET:
            *(int *)arg = SIO_MODE_INT | SIO_MODE_POLL;
            return (OK);


        case SIO_HW_OPTS_SET:
            status = (i8250OptsSet (pChan, arg) == OK) ? OK : EIO;
            break;


        case SIO_HW_OPTS_GET:
            *(int *)arg = pChan->options;
            break;


        case SIO_HUP:
            /* check if hupcl option is enabled */


            if (pChan->options & HUPCL) 
                status = i8250Hup (pChan);
            break;
        
        case SIO_OPEN:
            /* check if hupcl option is enabled */


            if (pChan->options & HUPCL) 
                status = i8250Open (pChan);
            break;


        default:
            status = ENOSYS;
            break;
        }


    return (status);
    }

/*******************************************************************************
*
* i8250HandleInterrupt - handle a receiver/transmitter interrupt
*
* This routine handles four sources of interrupts from the UART.  If there is
* another character to be transmitted, the character is sent.  When a modem
* status interrupt occurs, the transmit interrupt is enabled if the CTS signal
* is TRUE.
*
* INTERNAL
* The UART interrupts are prioritized in the following order by the Interrupt
* Identification Register:
*
*     Bit 2    bit 1    bit 0    Priority        Interrupt ID
*     -------------------------------------------------------
*     0        0        1        none            none
*     1        1        0        0               serialization error or BREAK
*     1        0        0        1               received data
*     0        1        0        2               transmitter buffer empty
*     0        0        0        3               RS-232 input
*
* In the table, priority level 0 is the highest priority.  These priorities
* are not typically configurable via software.  The interrupt is expressed as
* a two-bit integer.  Bit 0, the pending bit, is negative logic - a value 0
* means that an interrupt is pending.
*
* When nested interrupts occur, the second interrupt is identified in the
* Interrupt Identification Register (IIR) as soon as the first interrupt is
* cleared. Therefore, the interrupt service routine must continue to read IIR
* until bit-0 is 1.
*
* When the 8250 generates an interrupt, all interrupts with an equal or
* lower priority are locked out until the current interrupt has been cleared.
* The operation required to clear an interrupt varies according to the source.
* The actions typically required to clear an interrupt are as follows:
*
*    Source of interrupt                Response required to reset
*    ---------------------------------------------------------------------
*    receiver error or BREAK            read serialization status register
*    received data                      read data from receiver register
*    transmit buffer empty              write to the transmitter or read
*                                       the interrupt ID register
*    RS-232 input                       read the RS-232 status register
*
* In response to an empty transmit buffer (TBE), the interrupt can be
* cleared by writing a byte to the transmitter buffer register.  It can
* also be cleared by reading the interrupt identification register.
*
* Failure to clear all interrupts before returning will leave an interrupt
* pending and prevent the UART from generating further interrupt requests
* to the CPU.
*
* RETURNS: N/A
*/
static void i8250HandleInterrupt
    (
    I8250_CHAN  *  pChan
    )
    {
    char outChar;       /* store a byte for transmission */
    char interruptID;   /* store contents of interrupt ID register */
    char lineStatus;    /* store contents of line status register */
    char ier;           /* store contents of interrupt enable register */
    int  ix = 0;        /* allows us to return just in case IP never clears */

    ier = (*pChan->inByte) (pChan->ier);

    /* service UART interrupts until IP bit set or read counter expires */

    FOREVER
        {
        interruptID = ((*pChan->inByte)(pChan->iid) & I8250_IIR_MASK);

        if ((interruptID == I8250_IIR_IP) || (++ix > I8250_IIR_READ_MAX))
            {
            break;  /* interrupt cleared or loop counter expired */ 
            }


        /* filter possible anomalous interrupt ID from PC87307VUL (SPR 26117) */

        interruptID &= 0x06;  /* clear odd-bit to find interrupt ID */


        if (interruptID == I8250_IIR_SEOB)
            {
            lineStatus = (*pChan->inByte) (pChan->lst);
            }
        else if (interruptID == I8250_IIR_RBRF)
            {
            if (pChan->putRcvChar != NULL)
                (*pChan->putRcvChar)(pChan->putRcvArg,
                               (*pChan->inByte) (pChan->data));
            else
                (*pChan->inByte) (pChan->data);
            }
        else if (interruptID == I8250_IIR_THRE)
            {
            if ((pChan->getTxChar != NULL) &&
                (*pChan->getTxChar) (pChan->getTxArg, &outChar) == OK
               )
                {
                (*pChan->outByte) (pChan->data, outChar);
                }

            /* There are no bytes available for transmission.  Reading
             * the IIR at the top of this loop will clear the interrupt.
             */

            }
        else if (interruptID == I8250_IIR_MSTAT) /* modem status register */
            {
            char msr = (*(pChan)->inByte) (pChan->msr);

            /* (|=) ... DON'T CLOBBER BITS ALREADY SET IN THE IER */

            ier |= (I8250_IER_RXRDY | I8250_IER_MSI);

            /* if CTS is asserted by modem, enable tx interrupt */

            if ((msr & I8250_MSR_CTS) && (msr & I8250_MSR_DCTS))
                {
                (*pChan->outByte) (pChan->ier, (I8250_IER_TBE | ier));
                }
            else  /* turn off TBE interrupt until CTS from modem */
                {
                (*pChan->outByte) (pChan->ier, (ier & (~I8250_IER_TBE)));
                }
            }


        }  /* FOREVER */
    }

int	maxI8250Retry = 0;

/*******************************************************************************
*
* i8250Int - main interrupt routine for serial port interrupts
*
* RETURNS: N/A
*/
void i8250Int
    (
    I8250_CHAN  *  pChan
    )
{
	I8250_CHAN	 *  pCurrentChan;
	unsigned char	 retry;
	unsigned int	 pendingInterruptExists;

	/*
	 * Process all ports linked to this interrupt
	 */
	retry = 0;
	do
	{
		retry += 1;
		pCurrentChan = pChan;
		while ( pCurrentChan )
		{
			i8250HandleInterrupt(pCurrentChan);
			pCurrentChan = pCurrentChan->intChain;
	   }

		pendingInterruptExists = 0;
		pCurrentChan = pChan;
		while ( pCurrentChan && !pendingInterruptExists )
		{
        unsigned char interruptID = ((*pChan->inByte)(pChan->iid) & I8250_IIR_MASK);
        if ((interruptID != I8250_IIR_IP))
            {
            pendingInterruptExists = 1;
            }

			pCurrentChan = pCurrentChan->intChain;
	   }
		
	} while ( pendingInterruptExists && retry <= 3 );

	if ( retry > maxI8250Retry )
	{
		maxI8250Retry = retry;
	}
}

/*******************************************************************************
*
* i8250Startup - transmitter startup routine
*
* Call interrupt level character output routine and enable interrupt if it is
* in interrupt mode with no hardware flow control.
* If the option for hardware flow control is enabled and CTS is set TRUE,
* then the Tx interrupt is enabled.
*
* RETURNS: OK
*/
static int i8250Startup
    (
    I8250_CHAN *  pChan         /* tty device to start up */
    )
    {
    char  ier = I8250_IER_RXRDY;
    char  mask;


    if (pChan->channelMode == SIO_MODE_INT)
        {
        if (pChan->options & CLOCAL)
            {
            /* No modem control */


            ier |= I8250_IER_TBE;
            }
        else
            {
            mask = ((*pChan->inByte) (pChan->msr)) & I8250_MSR_CTS;


            /* if the CTS is asserted enable Tx interrupt */


            if (mask & I8250_MSR_CTS)
				    {
                ier |= (I8250_IER_TBE | I8250_IER_MSI);
					 }
            else
				    {
                ier |= (I8250_IER_MSI);
					 }
            }


        (*pChan->outByte) (pChan->ier, ier);
        }


    return (OK);
    }

/******************************************************************************
*
* i8250PRxChar - poll the device for input.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
* if the input buffer if empty.
*/
static int i8250PRxChar
    (
    I8250_CHAN *    pChan,      /* pointer to channel */
    char *          pChar       /* pointer to char */
    )
    {
    char pollStatus;


    pollStatus = ( *(pChan)->inByte) (pChan->lst);


    if ((pollStatus & I8250_LSR_RXRDY) == 0x00)
        return (EAGAIN);


    /* got a character */


    *pChar = ( *(pChan)->inByte) (pChan->data);


    return (OK);
    }

/******************************************************************************
*
* i8250PTxChar - output a character in polled mode.
* 
* Checks if the transmitter is empty. If empty, a character is written to
* the data register. 
*
* If the hardware flow control is enabled the handshake signal CTS has to be
* asserted in order to send a character.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
* if the output buffer if full.
*/
static int i8250PTxChar
    (
    I8250_CHAN *  pChan,        /* pointer to channel */
    char          outChar       /* char to send */
    )
    {
    char pollStatus;
    char msr;


    pollStatus = ( *(pChan)->inByte) (pChan->lst);


    msr = (*(pChan)->inByte) (pChan->msr);


    /* is the transmitter ready to accept a character? */


    if ((pollStatus & I8250_LSR_TEMT) == 0x00)
        return (EAGAIN);


    if (!(pChan->options & CLOCAL))      /* modem flow control ? */
        {
        if (msr & I8250_MSR_CTS)
            (*(pChan)->outByte) ((pChan)->data, outChar);
        else
            return (EAGAIN);
        }
    else
        (*(pChan)->outByte) ((pChan)->data, outChar);


    return (OK);
    }

STATUS setupSerialPort(
		unsigned int baseAddress,
		unsigned int intLevel
		)
{
	I8250_CHAN *	pChan;
	I8250_CHAN *	pLastChan = NULL;
	I8250_CHAN *	pNewChan;
	unsigned int	intVector = INT_VEC_GET(intLevel);
	char 				tyName[20];

	if ( !chanAccess )
	{
		chanAccess = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
	}

	/*
	 *	Make sure there isn't an existing port with the specified base address
	 */
	semTake(chanAccess, WAIT_FOREVER);
	pChan = pFirstChan;
	while ( pChan )
	{
		if ( pChan->baseAddress == baseAddress )
		{
			semGive(chanAccess);
			return ERROR;
		}

		pLastChan = pChan;
		pChan = pChan->portChain;
	}

	/*
	 *	Create a new data structure for the port and add it to the list of
	 * existing serial ports
	 */
	pNewChan = (I8250_CHAN *)malloc(sizeof(I8250_CHAN));
	memset((void *)pNewChan, 0, sizeof(I8250_CHAN));

	if ( pLastChan )
	{
		pLastChan->portChain = pNewChan;
	}
	else
	{
		pFirstChan = pNewChan;
	}

	/*
	 *	Intialize the serial port data and serial port hardware
	 */
	pNewChan->int_vec = intVector;
	pNewChan->channelMode = 0;
	pNewChan->lcr =  baseAddress + UART_LCR;
	pNewChan->data = baseAddress + UART_RDR;
	pNewChan->brdl = baseAddress + UART_BRDL;
	pNewChan->brdh = baseAddress + UART_BRDH;
	pNewChan->ier =  baseAddress + UART_IER;
	pNewChan->iid =  baseAddress + UART_IID;
	pNewChan->mdc =  baseAddress + UART_MDC;
	pNewChan->lst =  baseAddress + UART_LST;
	pNewChan->msr =  baseAddress + UART_MSR;

	pNewChan->outByte = sysOutByte;
	pNewChan->inByte  = sysInByte;

	i8250HrdInit(pNewChan);

	/*
	 *	Check if there is already a port attached to the requested interrupt
	 */
	pChan = pFirstChan;
	while ( pChan )
	{
		if ( pChan->int_vec == intVector &&
			  pChan != pNewChan )
		{
			break;
		}

		pChan = pChan->portChain;
	}

	if ( pChan )
	{
		/*
		 *	Interrupt already attached, add this port to the chain
		 */
		while ( pChan->intChain )
		{
			pChan = pChan->intChain;
		}

		pChan->intChain = pNewChan;
	}
	else
	{
		/*
		 *	First serial port on this interrupt
		 */
		intConnect(INUM_TO_IVEC(pNewChan->int_vec), i8250Int, (int)pNewChan);
		sysIntEnablePIC(intLevel);
	}

	/*
	 *	Create the device for this serial port
	 */
	sprintf(tyName, "/tyCo/%d", portNumber);
	portNumber += 1;
	ttyDevCreate(tyName, (SIO_CHAN *)pNewChan, 256, 256);

	semGive(chanAccess);
	return OK;
}
