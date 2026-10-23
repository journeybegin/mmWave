/*
 *  ======== main.c ========
 */

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* BIOS/XDC Include Files. */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/family/arm/v7a/Pmu.h>
#include <ti/sysbios/family/arm/v7r/vim/Hwi.h>

/* mmWave SDK Include Files: */
#include <ti/common/sys_common.h>
#include <ti/drivers/soc/soc.h>
#include <ti/drivers/esm/esm.h>
#include <ti/drivers/crc/crc.h>
#include <ti/drivers/uart/UART.h>
#include <ti/drivers/gpio/gpio.h>
#include <ti/drivers/pinmux/include/pinmux_xwr18xx.h>
#include <ti/drivers/pinmux/pinmux.h>
#include <ti/drivers/osal/DebugP.h>
#include <ti/drivers/adcbuf/ADCBuf.h>
#include <ti/control/mmwave/mmwave.h>

#include "mss_adcdata.h"
#include "signalDesign.h"

#pragma DATA_ALIGN(gAdcdataMSSMCB, 16)
Adcdata_MSS_MCB gAdcdataMSSMCB;
char       Total_frame;  //this variable can be used to set numFrame in function waitingCommand
/**
 *  @b Description
 *  @n
 *      This is the callback function registered with the ADC Driver which is invoked
 *      when a chirp is available. This is executed in the ISR context.
 *
 *  @param[in]  arg
 *      Application registered argument
 *
 *  @retval
 *      Not Applicable.
 */
//static void Adcdata_MSS_chirpIntCallback(uintptr_t arg)
//{
//
//}

/**
 *  @b Description
 *  @n
 *      This is the callback function registered with the ADC Driver which is invoked
 *      when a frame is started. This is executed in the ISR context.
 *
 *  @param[in]  arg
 *      Application registered argument
 *
 *  @retval
 *      Not Applicable.
 */
static void Adcdata_MSS_frameStartIntCallback(uintptr_t arg)
{
    gAdcdataMSSMCB.chirpCounter++;
    if(gAdcdataMSSMCB.chirpCounter > 1)
        if(gAdcdataMSSMCB.chirpCounter % 2 == 1)
                gAdcdataMSSMCB.frameCounter++;
    Semaphore_post(gAdcdataMSSMCB.uartSendSemHandle);
}

/**
 *  @b Description
 *  @n
 *      This is the task which provides an execution context for
 *      the mmWave control module.
 *
 *  @param[in]  arg0
 *      Argument0 with which the task was created
 *  @param[in]  arg1
 *      Argument1 with which the task was created
 *
 *  @retval
 *      Not Applicable.
 */
static void Adcdata_MSS_mmWaveCtrlTask (UArg arg0, UArg arg1)
{
    int32_t errCode;

    while (1)
    {
        /* Execute the mmWave control module: */
        if (MMWave_execute (gAdcdataMSSMCB.ctrlHandle, &errCode) < 0)
        {
            System_printf ("Error: mmWave control execution failed [Error code %d]\n", errCode);
        }
    }
}



/*
 *
 *
 *
 * */
static void Adcdata_MSS_UartSendTask (UArg arg0, UArg arg1)
{
    Adcdata_MessageHeader head;
    head.messageHead[0] = 0x1111;
    head.messageHead[1] = 0x2222;
    head.messageHead[2] = 0x1111;
    while (1)
    {

        Semaphore_pend(gAdcdataMSSMCB.uartSendSemHandle, BIOS_WAIT_FOREVER);
        UART_writePolling (gAdcdataMSSMCB.loggingUartHandle,
                                          (uint8_t*)&head,
                                          sizeof(Adcdata_MessageHeader));

        UART_writePolling (gAdcdataMSSMCB.loggingUartHandle,
                                           (uint8_t*)&gAdcdataMSSMCB.chirpCounter,
                                           sizeof(uint16_t));

        UART_writePolling (gAdcdataMSSMCB.loggingUartHandle,
                                           (uint8_t*)&gAdcdataMSSMCB.frameCounter,
                                           sizeof(uint16_t));


        UART_writePolling (gAdcdataMSSMCB.loggingUartHandle,
                                  (uint8_t*)gAdcdataMSSMCB.adcDataBuf,
                                  PROFILE_ADC_SAMPLE_VAL*4*4);

    }
}

/**
 *  @b Description
 *  @n
 *      Registered event function which is invoked when an event from the
 *      BSS is received.
 *
 *  @param[in]  msgId
 *      Message Identifier
 *  @param[in]  sbId
 *      Subblock identifier
 *  @param[in]  sbLen
 *      Length of the subblock
 *  @param[in]  payload
 *      Pointer to the payload buffer
 *
 *  @retval
 *      Always returns 0 [Continue passing the event to the peer domain]
 */
static int32_t Adcdata_MSS_eventFxn(uint16_t msgId, uint16_t sbId, uint16_t sbLen, uint8_t *payload)
{
    uint16_t asyncSB = RL_GET_SBID_FROM_UNIQ_SBID(sbId);

    /* Process the received message: */
    switch (msgId)
    {
        case RL_RF_ASYNC_EVENT_MSG:
        {
            /* Received Asychronous Message: */
            switch (asyncSB)
            {
                case RL_RF_AE_CPUFAULT_SB:
                {
                    /* Post event to datapath task notify BSS events */
                    //Event_post(gMrrMSSMCB.eventHandle, MMWDEMO_BSS_CPUFAULT_EVT);
                    break;
                }
                case RL_RF_AE_ESMFAULT_SB:
                {
                    /* Post event to datapath task notify BSS events */
                    //Event_post(gMrrMSSMCB.eventHandle, MMWDEMO_BSS_ESMFAULT_EVT);
                    break;
                }
                case RL_RF_AE_INITCALIBSTATUS_SB:
                {
                    /* This event should be handled by mmwave internally, ignore the event here */
                    break;
                }
//                case RL_RF_AE_FRAME_TRIGGER_RDY_SB:
//                {
//
//                    break;
//                }
                case RL_RF_AE_FRAME_END_SB:
                {
                    gAdcdataMSSMCB.runningStatus = false;
                    break;
                }
//                case RL_RF_AE_ANALOG_FAULT_SB:
//                {
//
//                    break;
//                }
                case RL_RF_AE_MON_TIMING_FAIL_REPORT_SB:
                {
                    break;
                }
                case RL_RF_AE_RUN_TIME_CALIB_REPORT_SB:
                {
                    break;
                }
                default:
                {
                    System_printf ("Error: Asynchronous Event SB Id %d not handled\n", asyncSB);
                    break;
                }
            }
            break;
        }
        default:
        {
           System_printf ("Error: Asynchronous message %d is NOT handled\n", msgId);
             break;
        }
    }
    return 0;
}


/**
 *  @b Description
 *  @n
 *      Function to configure ADCBUF driver based on CLI inputs.
 *  @param[out] numRxChannels Number of receive channels.
 *
 *  @retval
 *      Not Applicable.
 */
static int32_t Adcdata_ADCbufConfig()
{
    ADCBuf_dataFormat dataFormat;
    ADCBuf_RxChanConf rxChanConf;
    int32_t retVal;
    uint8_t channel;
    uint8_t adcFmt;
    uint8_t chInterleave;
    uint8_t sampleInterleave;
    uint32_t chirpThreshold;
    uint32_t rxChanMask = 0xF; /* All channels are enabled. */
    uint32_t offset;

    adcFmt = 0; /* Complex  mode */
    chInterleave = 1; /* Channel interleave mode is 1. i.e. no interleaving */
    sampleInterleave = 1; /* switch to Q first then I.  //Sample Interleave mode is I first then Q */
    chirpThreshold = 1;

    /* Divide the 32 kB ADC buffer into 4 parts. Assign each to one channel */
    offset = PROFILE_ADC_SAMPLE_VAL*sizeof(cmplx16ImRe_t);

    /*****************************************************************************
     * Data path :: ADCBUF driver Configuration
     *****************************************************************************/

    /* Populate data format from configuration */
    dataFormat.adcOutFormat = adcFmt;
    dataFormat.channelInterleave = chInterleave;
    dataFormat.sampleInterleave = sampleInterleave;

    /* Disable all ADCBuf channels */
    if ((retVal = ADCBuf_control(gAdcdataMSSMCB.adcBufHandle,
                                 ADCBufMMWave_CMD_CHANNEL_DISABLE,
                                 (void *) &rxChanMask)) < 0)
    {
        return retVal;
    }

    retVal = ADCBuf_control(gAdcdataMSSMCB.adcBufHandle,
                            ADCBufMMWave_CMD_CONF_DATA_FORMAT,
                            (void *) &dataFormat);
    if (retVal < 0)
    {
        return -1;
    }

    memset((void*) &rxChanConf, 0, sizeof(ADCBuf_RxChanConf));

    /* Enable all RX Channels */
    for (channel = 0; channel < SYS_COMMON_NUM_RX_CHANNEL; channel++)
    {
        if( RX_CHANNEL_ENABLE_ALL & ( 0x1U << channel ))
        {
            /* Populate the receive channel configuration: */
            rxChanConf.channel = channel;
            retVal = ADCBuf_control(gAdcdataMSSMCB.adcBufHandle,
                                    ADCBufMMWave_CMD_CHANNEL_ENABLE,
                                    (void *) &rxChanConf);

            if (retVal < 0)
            {
                return -1;
            }

            rxChanConf.offset += offset;
        }
    }

    /* Set ping-pong chirp threshold: */
    retVal = ADCBuf_control(gAdcdataMSSMCB.adcBufHandle,
                            ADCBufMMWave_CMD_SET_PING_CHIRP_THRESHHOLD,
                            (void *) &chirpThreshold);
    if (retVal < 0)
    {
        return -1;
    }
    retVal = ADCBuf_control(gAdcdataMSSMCB.adcBufHandle,
                            ADCBufMMWave_CMD_SET_PONG_CHIRP_THRESHHOLD,
                            (void *) &chirpThreshold);
    if (retVal < 0)
    {
        return -1;
    }

    return 0;
}



static void InitAdcBufDriver()
{
    {
        ADCBuf_Params adcBufParams;

        ADCBuf_init();
        /*****************************************************************************
         * Open ADCBUF driver:
         *****************************************************************************/
        ADCBuf_Params_init(&adcBufParams);
        adcBufParams.chirpThresholdPing = 1;
        adcBufParams.chirpThresholdPong = 1;
        adcBufParams.continousMode = 0;
        adcBufParams.socHandle = gAdcdataMSSMCB.socHandle;
        /* Open ADCBUF driver */
        gAdcdataMSSMCB.adcBufHandle = ADCBuf_open(0, &adcBufParams);
        if (gAdcdataMSSMCB.adcBufHandle == NULL)
        {
            System_printf("Error: Unable to open the ADCBUF driver\n");
            return;
        }
        Adcdata_ADCbufConfig();
    }
    return;
}


static void InitDriverTask (UArg arg0, UArg arg1)
{
    int32_t                 errCode;
    MMWave_InitCfg          initCfg;
    UART_Params             uartParams;
    Task_Params                taskParams;
    SOC_SysIntListenerCfg   listenerCfg;
    Semaphore_Params    semParams;

    //Debug Message:
    System_printf("Debug: Launched the Initialization Task\n");

    //PINMUX mapping
    Pinmux_Set_OverrideCtrl(SOC_XWR18XX_PINN5_PADBE, PINMUX_OUTEN_RETAIN_HW_CTRL, PINMUX_INPEN_RETAIN_HW_CTRL);
    Pinmux_Set_FuncSel(SOC_XWR18XX_PINN5_PADBE, SOC_XWR18XX_PINN5_PADBE_MSS_UARTA_TX);
    Pinmux_Set_OverrideCtrl(SOC_XWR18XX_PINN4_PADBD, PINMUX_OUTEN_RETAIN_HW_CTRL, PINMUX_INPEN_RETAIN_HW_CTRL);
    Pinmux_Set_FuncSel(SOC_XWR18XX_PINN4_PADBD, SOC_XWR18XX_PINN4_PADBD_MSS_UARTA_RX);

    /* Setup the PINMUX to bring out the UART-3 */
    Pinmux_Set_OverrideCtrl(SOC_XWR18XX_PINF14_PADAJ, PINMUX_OUTEN_RETAIN_HW_CTRL, PINMUX_INPEN_RETAIN_HW_CTRL);
    Pinmux_Set_FuncSel(SOC_XWR18XX_PINF14_PADAJ, SOC_XWR18XX_PINF14_PADAJ_MSS_UARTB_TX);

    /* Initialize the UART */
    UART_init();

    /* Initialize the GPIO */
    GPIO_init();

    /* Initialize the Mailbox*/
    Mailbox_init(MAILBOX_TYPE_MSS);

    /* Open the UART*/
    UART_Params_init(&uartParams);
    uartParams.clockFrequency = MSS_SYS_VCLK;
    uartParams.baudRate       = 115200;
    uartParams.isPinMuxDone   = 1;

    /* Open the Command UART Instance */
    gAdcdataMSSMCB.commandUartHandle = UART_open(0, &uartParams);
    if (gAdcdataMSSMCB.commandUartHandle == NULL)
    {
         System_printf("Error: Unable to open the Command UART Instance\n");
        return;
    }

    /* Setup the default UART Parameters */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.clockFrequency = MSS_SYS_VCLK;
    uartParams.baudRate       = 115200*8;
    uartParams.isPinMuxDone   = 1U;

    /* Open the Logging UART Instance: */
    gAdcdataMSSMCB.loggingUartHandle = UART_open(1, &uartParams);
    if (gAdcdataMSSMCB.loggingUartHandle == NULL)
    {
        System_printf("Error: MMWDemoMSS Unable to open the Logging UART Instance\n");
        return;
    }
    /* Initialize the ADCBuf*/
    InitAdcBufDriver();
    gAdcdataMSSMCB.adcDataBuf = (cmplx16ImRe_t *)SOC_XWR18XX_MSS_ADCBUF_BASE_ADDRESS;

    System_printf("Debug: ADCBuf Initialization\n");

    /* Register Chirp Available Listener */
//    memset ((void*)&listenerCfg, 0, sizeof(SOC_SysIntListenerCfg));
//    listenerCfg.systemInterrupt   = SOC_XWR18XX_MSS_CHIRP_AVAIL_IRQ;
//    listenerCfg.listenerFxn       = Adcdata_MSS_chirpIntCallback;
//    listenerCfg.arg               = 0;
//    gAdcdataMSSMCB.chirpIntHandle = SOC_registerSysIntListener (gAdcdataMSSMCB.socHandle, &listenerCfg, &errCode);
//    if (gAdcdataMSSMCB.chirpIntHandle == NULL)
//    {
//        System_printf ("Error: Unable to register the Chirp Available Listener [Error code %d]\n", errCode);
//        return;
//    }

    /* Register Frame Start Listener */
    memset ((void*)&listenerCfg, 0, sizeof(SOC_SysIntListenerCfg));
    listenerCfg.systemInterrupt   = SOC_XWR18XX_MSS_FRAME_START_INT;
    listenerCfg.listenerFxn       = Adcdata_MSS_frameStartIntCallback;
    listenerCfg.arg               = 0;
    gAdcdataMSSMCB.frameStartIntHandle = SOC_registerSysIntListener (gAdcdataMSSMCB.socHandle, &listenerCfg, &errCode);

    if (gAdcdataMSSMCB.frameStartIntHandle == NULL)
    {
        System_printf("Error: Unable to register the Frame start Listener [Error code %d]\n", errCode);
        return ;
    }
    System_printf ("Debug: frame and chirp interrupt initialized\n");

    /* Initialize the mmWave module*/
    memset ((void *)&initCfg, 0, sizeof(MMWave_InitCfg));

    initCfg.domain = MMWave_Domain_MSS;
    initCfg.socHandle = gAdcdataMSSMCB.socHandle;
    initCfg.eventFxn = &Adcdata_MSS_eventFxn;
    initCfg.cfgMode = MMWave_ConfigurationMode_MINIMAL;
    initCfg.executionMode = MMWave_ExecutionMode_ISOLATION;
    initCfg.linkCRCCfg.useCRCDriver = 1U;
    initCfg.linkCRCCfg.crcChannel = CRC_Channel_CH1;

    /* Initialize and setup the mmWave Control module */
    gAdcdataMSSMCB.ctrlHandle = MMWave_init (&initCfg, &errCode);
    if (gAdcdataMSSMCB.ctrlHandle == NULL)
    {
        /* Error: Unable to initialize the mmWave control module */
        System_printf ("Error: mmWave Control Initialization failed [Error code %d]\n", errCode);
        return;
    }
    System_printf ("Debug: Initialized the mmWave module\n");

    while (1)
    {
        int32_t syncStatus;

        /* Get the synchronization status: */
        syncStatus = MMWave_sync (gAdcdataMSSMCB.ctrlHandle, &errCode);
        if (syncStatus < 0)
        {
            /* Error: Unable to synchronize the mmWave control module */
            System_printf ("Error: mmWave Control Synchronization failed [Error code %d]\n", errCode);
            return;
        }
        if (syncStatus == 1)
        {
            /* Synchronization achieved: */
            break;
        }
        /* Sleep and poll again: */
        Task_sleep(1);
    }

    System_printf ("Debug: Synchronized the mmWave module\n");

    /* Create a binary semaphore which is used to handle UART Send data interrupt */
    Semaphore_Params_init(&semParams);
    semParams.mode             = Semaphore_Mode_BINARY;
    gAdcdataMSSMCB.uartSendSemHandle = Semaphore_create(0, &semParams, NULL);
    System_printf ("Debug: uartSendSemHandle SemHandles are created\n");
    /*****************************************************************************
     * Launch the mmWave control execution task
     * - This should have a higher priority than any other task which uses the
     *   mmWave control API
     *****************************************************************************/
    Task_Params_init(&taskParams);
    taskParams.priority  = 5;
    taskParams.stackSize = 3*1024;
    Task_create(Adcdata_MSS_mmWaveCtrlTask, &taskParams, NULL);//MMWave_execute线程，固定用法
    System_printf ("Debug: Adcdata_MSS_mmWaveCtrlTask is created\n");

    /*
     * Launch the Uart Send data Task
     * */
    Task_Params_init(&taskParams);
    taskParams.priority = 4;
    taskParams.stackSize = 4*1024;
    Task_create(Adcdata_MSS_UartSendTask, &taskParams, NULL);//串口线程

    Adcdata_MSS_Start ();
}


/*
 *  ======== main ========
 */
Int main()
{
    Task_Params        taskParams;
    int32_t         errCode;
    SOC_Cfg         socCfg;
    /* Initialize the ESM: Dont clear errors as TI RTOS does it */
    ESM_init(0U);

    /* Initialize the global variables */
    memset ((void*)&gAdcdataMSSMCB, 0, sizeof(Adcdata_MSS_MCB));

    /* Initialize the SOC configuration: */
    memset ((void *)&socCfg, 0, sizeof(SOC_Cfg));

    /* Populate the SOC configuration: */
    socCfg.clockCfg = SOC_SysClock_INIT;
    socCfg.dssCfg   = SOC_DSSCfg_HALT;
    socCfg.mpuCfg   = SOC_MPUCfg_CONFIG;
    /* Initialize the SOC Module: This is done as soon as the application is started
     * to ensure that the MPU is correctly configured. */
    gAdcdataMSSMCB.socHandle = SOC_init (&socCfg, &errCode);
    if (gAdcdataMSSMCB.socHandle == NULL)
    {
        System_printf ("Error: SOC Module Initialization failed [Error code %d]\n", errCode);
        return -1;
    }

    /* Check if the SOC is a secure device */
    if (SOC_isSecureDevice(gAdcdataMSSMCB.socHandle, &errCode))
    {
        /* Disable fire wall for JTAG and LOGGER (UART) which is needed by the demo */
        SOC_controlSecureFirewall(gAdcdataMSSMCB.socHandle,
                                  (uint32_t)(SOC_SECURE_FIREWALL_JTAG | SOC_SECURE_FIREWALL_LOGGER),
                                  SOC_SECURE_FIREWALL_DISABLE,
                                  &errCode);
    }

    System_printf ("Success: SOC Module Initialization successed\n");

    /* Initialize the Task Parameters. */
    Task_Params_init(&taskParams);
    taskParams.priority = 1;
    Task_create(InitDriverTask, &taskParams, NULL);

    /* Start BIOS */
    BIOS_start();
    return 0;
}


