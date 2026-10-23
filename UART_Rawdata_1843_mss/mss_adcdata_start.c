/*
 * mss_adcdata_cli.c
 *
 *  Created on: 2021Äê12ÔÂ20ÈÕ
 *      Author: ShenJie
 */

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/knl/Event.h>

#include <ti/common/sys_common.h>
#include <ti/drivers/uart/UART.h>
#include <ti/drivers/osal/DebugP.h>
#include <ti/control/mmwavelink/mmwavelink.h>
#include <ti/utils/cli/cli.h>

#include "mss_adcdata.h"
#include "signalDesign.h"

char waitingCommand();
static int32_t Adcdata_MSS_AdvancedFrameCfg ();
static int32_t Adcdata_MSS_SensorStart ();
static void Cfg_ChannelCfgInitParams (rlChanCfg_t* ptrChannelCfg);
static void Cfg_ADCOutCfgInitParams (rlAdcOutCfg_t* ptrADCOutCfg);
static void Cfg_LowPowerModeInitParams (rlLowPowerModeCfg_t* ptrLowPowerMode);
static void Cfg_ProfileCfgInitParams (rlProfileCfg_t* ptrProfileCfg);
static void Cfg_ChirpCfgInitParams (uint8_t chirpnum,rlChirpCfg_t* ptrChirpCfg);
static void Cfg_AdvFrameCfgInitParams (rlAdvFrameCfg_t* ptrFrameCfg);
//static void Cfg_FrameCfgInitParams (rlFrameCfg_t* ptrFrameCfg);

extern char       Total_frame;
extern uint16_t   chirpCounter;
extern uint16_t   frameCounter;

char waitingCommand()
{
    uint8_t  cmdString[256];
    char     *ptrCLICommand;
    char     *tokenizedArgs[2];
    char     delimitter[]      = " \r\n";
    uint32_t argIndex;
    char     *command          = "sensorStart";
    char     framedefault      = FRAME_Adcdata_NUMBER_COUNT;

    do
    {
        memset((void *)&cmdString[0], 0, sizeof(cmdString));
        UART_read(gAdcdataMSSMCB.commandUartHandle, &cmdString[0], (sizeof(cmdString) - 1));
        argIndex = 0;
        memset((void *)&tokenizedArgs, 0, sizeof(tokenizedArgs));
        ptrCLICommand = (char *)&cmdString[0];
        while (1)
        {
            /* Tokenize the arguments: */
            tokenizedArgs[argIndex] = strtok(ptrCLICommand, delimitter);
            if (tokenizedArgs[argIndex] == NULL)
                break;

            /* Increment the argument index: */
            argIndex++;
            if (argIndex >= 2)
                break;

            /* Reset the command string */
            ptrCLICommand = NULL;
        }

    }while(strcmp(command,tokenizedArgs[0]) != 0);
    if (tokenizedArgs[1] == NULL)
        tokenizedArgs[1] = &framedefault;
    gAdcdataMSSMCB.startRequest = true;

    return *tokenizedArgs[1];

}

//static void Cfg_FrameCfgInitParams (rlFrameCfg_t* ptrFrameCfg)
//{
//
//    /* Initialize the configuration: */
//    memset ((void*)ptrFrameCfg, 0, sizeof(rlFrameCfg_t));
//
//    /* Populate the default configuration: */
//    ptrFrameCfg->chirpStartIdx     = CHIRP_Adcdata_START_0_INDEX;
//    ptrFrameCfg->chirpEndIdx       = CHIRP_Adcdata_END_1_INDEX;
//    ptrFrameCfg->numLoops          = FRAME_Adcdata_LOOP_COUNT;
//    ptrFrameCfg->numFrames         = FRAME_Adcdata_NUMBER_COUNT;
//    ptrFrameCfg->framePeriodicity  = FRAME_Adcdata_PERIODICITY_VAL;
//    ptrFrameCfg->triggerSelect     = 1;
//    ptrFrameCfg->frameTriggerDelay = 0;
//    return;
//}


static void Cfg_AdvFrameCfgInitParams (rlAdvFrameCfg_t* ptrAdvFrameCfg)
{
    uint8_t numOfSubFrame = 0U;

    /* Initialize the configuration: */
    memset ((void*)ptrAdvFrameCfg, 0, sizeof(rlAdvFrameCfg_t));

    /* Populate the default configuration: */
    ptrAdvFrameCfg->frameSeq.forceProfile  = 0;// 0: force Profile,  1: Don't force profile
    ptrAdvFrameCfg->frameSeq.numFrames     = SUBFRAME_Adcdata_NUMBER_COUNT;//finite
    ptrAdvFrameCfg->frameSeq.triggerSelect = 1;//SW Trigger
    ptrAdvFrameCfg->frameSeq.frameTrigDelay= 0;

    /* The high resolution 20m subframe */
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].forceProfileIdx    = 0;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].numLoops           = SUBFRAME_Adcdata_LOOP_COUNT;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].numOfBurst         = 1;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].numOfBurstLoops    = 1;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].chirpStartIdxOffset= 0;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].numOfChirps        = SUBFRAME_Adcdata_CHIRPIDX_EACH_BURST;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].chirpStartIdx      = CHIRP_Adcdata_START_0_INDEX;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].subFramePeriodicity= SUBFRAME_Adcdata_PERIODICITY_VAL;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[0].burstPeriodicity   = SUBFRAME_Adcdata_PERIODICITY_VAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[0].numAdcSamples = PROFILE_Adcdata_ADC_SAMPLE_VAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[0].totalChirps   = SUBFRAME_Adcdata_NUM_CHIRPS_TOTAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[0].numChirpsInDataPacket = 1;
    numOfSubFrame++;

    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].forceProfileIdx    = 0;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].numLoops           = SUBFRAME_Adcdata_LOOP_COUNT;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].numOfBurst         = 1;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].numOfBurstLoops    = 1;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].chirpStartIdxOffset= 0;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].numOfChirps        = SUBFRAME_Adcdata_CHIRPIDX_EACH_BURST;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].chirpStartIdx      = CHIRP_Adcdata_START_1_INDEX;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].subFramePeriodicity= SUBFRAME_Adcdata_PERIODICITY_VAL;
    ptrAdvFrameCfg->frameSeq.subFrameCfg[1].burstPeriodicity   = SUBFRAME_Adcdata_PERIODICITY_VAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[1].numAdcSamples = PROFILE_Adcdata_ADC_SAMPLE_VAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[1].totalChirps   = SUBFRAME_Adcdata_NUM_CHIRPS_TOTAL;
    ptrAdvFrameCfg->frameData.subframeDataCfg[1].numChirpsInDataPacket = 1;
    numOfSubFrame++;
    ptrAdvFrameCfg->frameSeq.numOfSubFrames = numOfSubFrame;
    ptrAdvFrameCfg->frameData.numSubFrames  = numOfSubFrame;
    return;
}

static void Cfg_ChirpCfgInitParams (uint8_t chirpnum, rlChirpCfg_t* ptrChirpCfg)
{
    /* Initialize the configuration: */
    memset ((void*)ptrChirpCfg, 0, sizeof(rlChirpCfg_t));

    if(chirpnum == 0U)
    {
        /* Populate the default configuration for chirp 0.
         *  - MRR Fast Chirp for max-velocity enhancement. */
        ptrChirpCfg->profileId       = CHIRP_Adcdata_PROFILE_ID;
        ptrChirpCfg->adcStartTimeVar = CHIRP_Adcdata_ADC_START_TIME_VAL;
        ptrChirpCfg->chirpStartIdx   = CHIRP_Adcdata_START_0_INDEX;
        ptrChirpCfg->chirpEndIdx     = CHIRP_Adcdata_END_0_INDEX;
        ptrChirpCfg->idleTimeVar     = CHIRP_Adcdata_IDLE_TIME_VAL;
        ptrChirpCfg->txEnable        = CHIRP_Adcdata_TX0_CHANNEL;
        ptrChirpCfg->startFreqVar    = CHIRP_Adcdata_START_FREQ_VAL;
        ptrChirpCfg->freqSlopeVar    = CHIRP_Adcdata_FREQ_SLOPE_VAL;
    }

    else if(chirpnum == 1U)
    {
        /* Populate the default configuration for chirp 1.
         *  - MRR Fast Chirp for max-velocity enhancement. */
        ptrChirpCfg->profileId       = CHIRP_Adcdata_PROFILE_ID;
        ptrChirpCfg->adcStartTimeVar = CHIRP_Adcdata_ADC_START_TIME_VAL;
        ptrChirpCfg->chirpStartIdx   = CHIRP_Adcdata_START_1_INDEX;
        ptrChirpCfg->chirpEndIdx     = CHIRP_Adcdata_END_1_INDEX;
        ptrChirpCfg->idleTimeVar     = CHIRP_Adcdata_IDLE_TIME_VAL;
        ptrChirpCfg->txEnable        = CHIRP_Adcdata_TX2_CHANNEL;
        ptrChirpCfg->startFreqVar    = CHIRP_Adcdata_START_FREQ_VAL;
        ptrChirpCfg->freqSlopeVar    = CHIRP_Adcdata_FREQ_SLOPE_VAL;
    }

    return;
}

static void Cfg_ProfileCfgInitParams (rlProfileCfg_t* ptrProfileCfg)
{
    /* Initialize the configuration: */
    memset ((void*)ptrProfileCfg, 0, sizeof(rlProfileCfg_t));


    /* Populate the default configuration for profile 0  */
    ptrProfileCfg->profileId             = PROFILE_Adcdata_PROFILE_ID;
    ptrProfileCfg->pfVcoSelect           = 0;
    ptrProfileCfg->pfCalLutUpdate        = 0;
    ptrProfileCfg->startFreqConst        = PROFILE_Adcdata_START_FREQ_VAL;
    ptrProfileCfg->idleTimeConst         = PROFILE_Adcdata_IDLE_TIME_VAL;
    ptrProfileCfg->adcStartTimeConst     = PROFILE_Adcdata_ADC_START_TIME_VAL;
    ptrProfileCfg->rampEndTime           = PROFILE_Adcdata_RAMP_END_TIME_VAL;
    ptrProfileCfg->txOutPowerBackoffCode = PROFILE_Adcdata_TXOUT_POWER_BACKOFF;
    ptrProfileCfg->txPhaseShifter        = PROFILE_Adcdata_TXPHASESHIFTER_VAL;
    ptrProfileCfg->freqSlopeConst        = PROFILE_Adcdata_FREQ_SLOPE_VAL;
    ptrProfileCfg->txStartTime           = PROFILE_Adcdata_TX_START_TIME_VAL;
    ptrProfileCfg->numAdcSamples         = PROFILE_Adcdata_ADC_SAMPLE_VAL;
    ptrProfileCfg->digOutSampleRate      = PROFILE_Adcdata_DIGOUT_SAMPLERATE_VAL;
    ptrProfileCfg->hpfCornerFreq1        = PROFILE_Adcdata_HPFCORNER_FREQ1_VAL;
    ptrProfileCfg->hpfCornerFreq2        = PROFILE_Adcdata_HPFCORNER_FREQ2_VAL;
    ptrProfileCfg->txCalibEnCfg          = 0;
    ptrProfileCfg->rxGain                = PROFILE_Adcdata_RX_GAIN_VAL;


    return;
}



static void Cfg_LowPowerModeInitParams (rlLowPowerModeCfg_t* ptrLowPowerMode)
{
    /* Initialize the configuration: */
    memset ((void*)ptrLowPowerMode, 0, sizeof(rlLowPowerModeCfg_t));

    /* Populate the low power configuration: */
    ptrLowPowerMode->lpAdcMode     = LP_ADC_MODE_REGULAR;
    return;
}


static void Cfg_ChannelCfgInitParams (rlChanCfg_t* ptrChannelCfg)
{
    /* Initialize the configuration: */
    memset ((void*)ptrChannelCfg, 0, sizeof(rlChanCfg_t));

    /* Populate the default configuration: */
    ptrChannelCfg->rxChannelEn = RX_CHANNEL_ENABLE_ALL; //all Rx_antennas enable
    ptrChannelCfg->txChannelEn = TX_CHANNEL_ENABLE_2D;//Tx0 and Tx2 enable;
    ptrChannelCfg->cascading   = 0; /* Single Chip (no cascading)*/

    return;
}

static void Cfg_ADCOutCfgInitParams (rlAdcOutCfg_t* ptrADCOutCfg)
{
    /* Initialize the configuration: */
    memset ((void*)ptrADCOutCfg, 0, sizeof(rlAdcOutCfg_t));

    /* Populate the default configuration: */
    ptrADCOutCfg->fmt.b2AdcBits   = ADC_BITS_16;
    ptrADCOutCfg->fmt.b2AdcOutFmt = ADC_FORMAT_COMPLEX;
    ptrADCOutCfg->fmt.b8FullScaleReducFctr = 0;

    return;
}

/**
 *  @b Description
 *  @n
 *      This is the CLI Handler for starting the sensor
 *
 *  @retval
 *      Success -   0
 *  @retval
 *      Error   -   <0
 */
static int32_t Adcdata_MSS_SensorStart ()
{
    MMWave_CalibrationCfg   calibrationCfg;
    int32_t                 errCode;
    char                    Total_frame;

    Total_frame = waitingCommand();
    if (gAdcdataMSSMCB.runningStatus == true)
    {
        /* Already running. */
        return (int32_t)Total_frame;
    }
    if(gAdcdataMSSMCB.startRequest == true)
    {
        /* The sensor can only be started; if the link has been configured */
        if (gAdcdataMSSMCB.cfgStatus == true)
        {
            /* Initialize the calibration configuration: */
            memset ((void *)&calibrationCfg, 0, sizeof(MMWave_CalibrationCfg));

            /* Populate the calibration configuration: */
//            calibrationCfg.dfeDataOutputMode                          =
//                                              MMWave_DFEDataOutputMode_FRAME;
            calibrationCfg.dfeDataOutputMode                          =
                                     MMWave_DFEDataOutputMode_ADVANCED_FRAME;
            calibrationCfg.u.chirpCalibrationCfg.enableCalibration    = true;
            calibrationCfg.u.chirpCalibrationCfg.enablePeriodicity    = true;
            calibrationCfg.u.chirpCalibrationCfg.periodicTimeInFrames = 10U;

            System_printf ("Debug: Sensor will start momentarily. \n");
            /* Start the mmWave: */
            if (MMWave_start (gAdcdataMSSMCB.ctrlHandle, &calibrationCfg, &errCode) < 0)
            {
                /* Error: Unable to start the mmWave control module */
                System_printf ("Error: mmWave start failed [Error code %d]\n", errCode);
                return -1;
            }
            gAdcdataMSSMCB.runningStatus = true;
            return 0;
        }
        else
        {
            /* Invalid CLI use case; doing a sensor start without executing the basic or advanced configuration
             * command. Inform the user and return an error code. */
            System_printf ("Error: Please ensure that the XXXCfg CLI command is invoked before starting the sensor\n");
            return -1;
        }
    }

    else
    {
        /* Invalid CLI use case; doing a sensor start without executing the basic or advanced configuration
         * command. Inform the user and return an error code. */
        System_printf ("Error: Please ensure that valid value of startRequest before starting the sensor\n");
        return -1;
    }
}

static int32_t Adcdata_MSS_AdvancedFrameCfg ()
{
    MMWave_OpenCfg          openCfg;
    int32_t                 errCode;
    rlProfileCfg_t          profileCfg;
    rlChirpCfg_t            chirpCfg;
    rlAdvFrameCfg_t         AdvFrameCfg;
//    rlFrameCfg_t            FrameCfg;
    rlRfLdoBypassCfg_t rfLdoBypassCfg[1] = {0};
    int32_t                 retVal;
    uint8_t                 indx;

//    Total_frame = waitingCommand();
    if (gAdcdataMSSMCB.cfgStatus == true)
    {
        /* Radar has already been configured. */
        return 0;
    }

    /* Setup the calibration frequency: */
    openCfg.freqLimitLow  = 760U;
    openCfg.freqLimitHigh = 810U;
    openCfg.defaultAsyncEventHandler = MMWave_DefaultAsyncEventHandler_MSS;
    openCfg.disableFrameStartAsyncEvent = 1;
    openCfg.calibMonTimeUnit = 1;

    /* Initialize the minimal configuration: */
    Cfg_ChannelCfgInitParams  (&openCfg.chCfg);
    Cfg_LowPowerModeInitParams(&openCfg.lowPowerMode);
    Cfg_ADCOutCfgInitParams   (&openCfg.adcOutCfg);

    /* Open the mmWave module: */
    if (MMWave_open (gAdcdataMSSMCB.ctrlHandle, &openCfg, NULL, &errCode) < 0)
    {
        System_printf ("Error: MMWDemoMSS mmWave open configuration failed [Error code %d]\n", errCode);
        return -1;
    }

    System_printf ("Set LDO Bypass\n");
    rfLdoBypassCfg->ldoBypassEnable = 0x03;
    retVal = rlRfSetLdoBypassConfig(RL_DEVICE_MAP_INTERNAL_BSS, (rlRfLdoBypassCfg_t*)rfLdoBypassCfg);
    if (retVal != RL_RET_CODE_OK)
    {
        System_printf ("Error: LDO Bypass config failed [Error %d]\n", retVal);
        return -1;
    }

    /********************************************************************************
     * MMWave Link and BSS is operational now. In minimal mode we have access to all
     * the mmWave Link API to perform the configuration
     *
     * Profile configurations:
     ********************************************************************************/
    Cfg_ProfileCfgInitParams (&profileCfg);
    retVal = rlSetProfileConfig (RL_DEVICE_MAP_INTERNAL_BSS, 1U, &profileCfg);
    if (retVal != RL_RET_CODE_OK)
    {
        System_printf ("Error: Unable to configure the profile [Error %d]\n", retVal);
        return -1;
    }

    /********************************************************************************
     * Chirp configurations:
     ********************************************************************************/
    for(indx = 0;indx < FRAME_Adcdata_CHIRPIDX;indx++)
    {
        Cfg_ChirpCfgInitParams (indx,&chirpCfg);
        retVal = rlSetChirpConfig(RL_DEVICE_MAP_INTERNAL_BSS, 1U, &chirpCfg);
        if (retVal != RL_RET_CODE_OK)
        {
            System_printf ("Error: Unable to configure chirp [Error %d]\n", retVal);
            return -1;
        }
    }

    /* Frame configuration. */
//    Cfg_FrameCfgInitParams (&FrameCfg);
////    FrameCfg.numFrames = (uint16_t)Total_frame;
//    retVal = rlSetFrameConfig(RL_DEVICE_MAP_INTERNAL_BSS, &FrameCfg);
//    if (retVal != RL_RET_CODE_OK)
//    {
//        System_printf ("Error: Frame configuration failed [Error %d]\n", retVal);
//        return -1;
//    }

    /* Advanced Frame configuration. */
    Cfg_AdvFrameCfgInitParams (&AdvFrameCfg);
//    AdvFrameCfg.frameSeq.numFrames = (uint16_t)Total_frame;
    retVal = rlSetAdvFrameConfig(RL_DEVICE_MAP_INTERNAL_BSS, &AdvFrameCfg);
    if (retVal != RL_RET_CODE_OK)
    {
        System_printf ("Error: Advanced Frame configuration failed [Error %d]\n", retVal);
        return -1;
    }

    /* The link has been configured. */
    gAdcdataMSSMCB.cfgStatus = true;
    System_printf ("Debug: MMWave has been configured for MRR.\n");
    return 0;
}

//int32_t Adcdata_MSS_Open(void)
//{
//    MMWave_OpenCfg          openCfg;
//    rlRfLdoBypassCfg_t rfLdoBypassCfg[1] = {0};
//    int32_t                 errCode;
//    int32_t                 retVal  = 0;
//
////    if (gAdcdataMSSMCB.cfgStatus == true)
////    {
////        /* Radar has already been configured. */
////        return 0;
////    }
//
//    /* Setup the calibration frequency: */
//    openCfg.freqLimitLow  = 760U;
//    openCfg.freqLimitHigh = 810U;
//    openCfg.defaultAsyncEventHandler = MMWave_DefaultAsyncEventHandler_MSS;
//    //    openCfg.calibMonTimeUnit = 1;
//
//    /* Initialize the minimal configuration: */
//    Cfg_ChannelCfgInitParams  (&openCfg.chCfg);
//    Cfg_LowPowerModeInitParams(&openCfg.lowPowerMode);
//    Cfg_ADCOutCfgInitParams   (&openCfg.adcOutCfg);
//
//    /* Open the mmWave module: */
//    if (MMWave_open (gAdcdataMSSMCB.ctrlHandle, &openCfg, NULL, &errCode) < 0)
//    {
//        System_printf ("Error: MMWDemoMSS mmWave open configuration failed [Error code %d]\n", errCode);
//        return -1;
//    }
//
//    System_printf ("Set LDO Bypass\n");
//    rfLdoBypassCfg->ldoBypassEnable = 0x03;
//    retVal = rlRfSetLdoBypassConfig(RL_DEVICE_MAP_INTERNAL_BSS, (rlRfLdoBypassCfg_t*)rfLdoBypassCfg);
//    if (retVal != RL_RET_CODE_OK)
//    {
//        System_printf ("Error: LDO Bypass config failed [Error %d]\n", retVal);
//        return -1;
//    }
//
//    return retVal;
//}

static void Adcdata_MSS_radarConfigTask (UArg arg0, UArg arg1)
{
    int32_t errCode;
    char   loopEndString[] = "sensorStop";
//    retVal = Adcdata_MSS_Open();
//    if(retVal == 0)
//    {
//        System_printf ("mmWave open configuration success\n");
//    }
    gAdcdataMSSMCB.cfgStatus     = false;
    Adcdata_MSS_AdvancedFrameCfg ();
    while(1)
    {
        gAdcdataMSSMCB.frameCounter = 1;
        gAdcdataMSSMCB.chirpCounter = 0;
        gAdcdataMSSMCB.startRequest  = false;
        gAdcdataMSSMCB.runningStatus = false;
        Adcdata_MSS_SensorStart ();
        while(gAdcdataMSSMCB.runningStatus == true);
        if(MMWave_stop(gAdcdataMSSMCB.ctrlHandle,&errCode) < 0)
        {
            System_printf ("Error: mmWave stop failed [Error code %d]\n", errCode);
        }
        Task_sleep(1);
        UART_writePolling (gAdcdataMSSMCB.commandUartHandle,
                           (uint8_t*)loopEndString,
                           sizeof(loopEndString));
    }
}

/**
 *  @b Description
 *  @n
 *      This is the CLI Execution Task
 *
 *  @retval
 *      Not Applicable.
 */
void Adcdata_MSS_Start (void)
{
    Task_Params taskParams;

    Task_Params_init(&taskParams);
    taskParams.priority = 3;
    taskParams.stackSize = 4*1024;
    Task_create(Adcdata_MSS_radarConfigTask, &taskParams, NULL);
    return;
}
