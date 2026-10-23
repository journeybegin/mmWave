/*
 * mss_adcdata.h
 *
 *  Created on: 2021Äê12ÔÂ20ÈÕ
 *      Author: ShenJie
 */

#ifndef MSS_ADCDATA_H_
#define MSS_ADCDATA_H_

#include <ti/drivers/mailbox/mailbox.h>
#include <ti/drivers/adcbuf/ADCBuf.h>
#include <ti/control/mmwave/mmwave.h>

typedef enum Message_type_e
{
    MSG_CHIRP_FRAME = 1,
    MSG_ADCDATA
}Message_type;

typedef struct Adcdata_MessageHeader_t
{
    uint16_t messageHead[3];

}Adcdata_MessageHeader;

typedef struct Adcdata_MessageTLV_t
{
    /*! @brief   TLV type */
    uint16_t type;
    /*! @brief   Length in bytes */
    uint32_t length;
}Adcdata_MessageTLV;


typedef struct Adcdata_MSS_MCB_t
{
    /*
     * Handle to the SOC Module
     * */
    SOC_Handle socHandle;

    /*
     * An token for frame start events
     * */
//    int32_t frameStartToken;

    /*
     * The number of subFrames transmitted derived from the frame start interrupts
     * */
//    int32_t subFrameCntFromFrameStart;

    /*
     * The number of Frames transmitted derived from the frame start interrupts
     * */
    uint16_t frameCounter;

    /*
     * The number of chirp transmitted derived from the frame start interrupts
     * */
    uint16_t chirpCounter;


    /*
     * An indicator for the current subframe
     * */
//    int32_t subframeId;

    /*
     * The number of subframes transmitted derived from chirp available interrupts
     * */
//    int32_t subframeCntFromChirpInt;

    /*
     * UART Command Handle used to interface with the CLI
     * */
    UART_Handle commandUartHandle;

    /*
     * UART Logging Handle
     * */
    UART_Handle loggingUartHandle;

    /*
     * Semaphore handle for the mailbox communication
     * */
    Semaphore_Handle mboxSemHandle;

    /*
     * Handle to the peer Mailbox
     * */
    Mbox_Handle peerMailbox;

    /*
     * Handle to the SOC chirp interrupt listener Handle
     * */
    SOC_SysIntListenerHandle chirpIntHandle;

    /*
     * Handle to the SOC chirp interrupt listener Handle
     * */
    SOC_SysIntListenerHandle frameStartIntHandle;

    /*
     * mmWave control handle use to initialize the link infrastructure
     * which allows communication between the MSS and BSS
     * */
    MMWave_Handle ctrlHandle;


    /*
     * MSS system event handle
     * */
    Event_Handle eventHandleNotify;

    /**
     * @brief   Handle to the ADCBUF Driver
     */
    ADCBuf_Handle adcBufHandle;

    /*
     * @brief   adcbuf start address
     * */
    cmplx16ImRe_t *adcDataBuf;

    /*
     * Semaphore handle for the Uart send data to PC
     * */
    Semaphore_Handle uartSendSemHandle;

    /**
     * @brief   This is a flag which indicates if the mmWave link has been configured
     * or not.
     */
    bool                        cfgStatus;

    /**
     * @brief   This is a flag which indicates if the radar is tranmsitting or not
     * or not.
     */
    bool                        runningStatus;

    /**
     * @brief   This is a flag which indicates if user want to start radar.
     */
    bool                        startRequest;
}Adcdata_MSS_MCB;


extern Adcdata_MSS_MCB gAdcdataMSSMCB;

extern void Adcdata_MSS_Start (void);
extern int32_t Adcdata_MSS_Open (void);

#endif /* MSS_ADCDATA_H_ */
