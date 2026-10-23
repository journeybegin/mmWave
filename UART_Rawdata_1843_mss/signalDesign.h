/*
 * signalDesign.h
 *
 *  Created on: 2021Äê12ÔÂ20ÈÕ
 *      Author: ShenJie
 */

#ifndef SIGNALDESIGN_H_
#define SIGNALDESIGN_H_

#define TX_CHANNEL_ENABLE_2D  (5U)
#define TX_CHANNEL_ENABLE_ALL  (7U)
#define RX_CHANNEL_ENABLE_ALL  (15U)
#define ADC_BITS_16 (2U)
#define ADC_FORMAT_COMPLEX (1U)

#define LP_ADC_MODE_REGULAR (0U)
#define LP_ADC_MODE_LOW_POWER (1U)

#define ROUND_TO_INT32(X)    ((int32_t) (X))
#define CONV_FREQ_GHZ_TO_CODEWORD(X) ROUND_TO_INT32(X * (1.0e9/53.644))
#define CONV_SLOPE_MHZ_PER_US_TO_CODEWORD(X) (ROUND_TO_INT32(X * (1000.0/48.279)))
/*
 * Profile setting
 * */

#define PROFILE_Adcdata_PROFILE_ID 0
#define PROFILE_Adcdata_START_FREQ_GHZ (77.00f)
#define PROFILE_Adcdata_START_FREQ_VAL (CONV_FREQ_GHZ_TO_CODEWORD(PROFILE_Adcdata_START_FREQ_GHZ))
#define PROFILE_Adcdata_IDLE_TIME_VAL (700U)
#define PROFILE_Adcdata_ADC_START_TIME_VAL (193U)
#define PROFILE_Adcdata_RAMP_END_TIME_VAL (8902U)
#define PROFILE_Adcdata_TXOUT_POWER_BACKOFF (0U)
#define PROFILE_Adcdata_TXPHASESHIFTER_VAL (0U)
#define PROFILE_Adcdata_FREQ_SLOPE_MHZ_PER_US (44.561f)
#define PROFILE_Adcdata_FREQ_SLOPE_VAL (CONV_SLOPE_MHZ_PER_US_TO_CODEWORD(PROFILE_Adcdata_FREQ_SLOPE_MHZ_PER_US))
#define PROFILE_Adcdata_TX_START_TIME_VAL (100U)
#define PROFILE_Adcdata_ADC_SAMPLE_VAL (256U)
#define PROFILE_Adcdata_DIGOUT_SAMPLERATE_VAL (3000U)
/*! @brief  HPF 1 corner frequency. */
#define PROFILE_Adcdata_HPFCORNER_FREQ1_VAL RL_RX_HPF1_175_KHz
/*! @brief  HPF 2 corner frequency. */
#define PROFILE_Adcdata_HPFCORNER_FREQ2_VAL RL_RX_HPF2_350_KHz
/*! @brief  Rx gain is kept at the maximum . */
#define PROFILE_Adcdata_RX_GAIN_VAL (44U)

/*
 * Chirp setting
 * */
#define CHIRP_Adcdata_PROFILE_ID (0U)
#define CHIRP_Adcdata_START_0_INDEX (0U)
#define CHIRP_Adcdata_END_0_INDEX (0U)
#define CHIRP_Adcdata_START_1_INDEX (1U)
#define CHIRP_Adcdata_END_1_INDEX (1U)
#define CHIRP_Adcdata_START_FREQ_VAL (0U)
#define CHIRP_Adcdata_FREQ_SLOPE_VAL (0U)
#define CHIRP_Adcdata_IDLE_TIME_VAL (0U)
#define CHIRP_Adcdata_ADC_START_TIME_VAL (0U)
#define CHIRP_Adcdata_TX0_CHANNEL (1U)
#define CHIRP_Adcdata_TX2_CHANNEL (4U)

/*
 * Frame setting
 * */
#define FRAME_Adcdata_LOOP_COUNT (2U)
#define FRAME_Adcdata_NUMBER_COUNT (8U)
#define FRAME_Adcdata_PERIODICITY_VAL (16000000U)
#define FRAME_Adcdata_CHIRPIDX (CHIRP_Adcdata_END_1_INDEX - CHIRP_Adcdata_START_0_INDEX + 1U)
#define FRAME_Adcdata_CHIRPS_ALL_LOOP (FRAME_Adcdata_CHIRPIDX * FRAME_Adcdata_LOOP_COUNT)
/*
 * Advanced frame setting
 * */
#define SUBFRAME_Adcdata_NUMBER_COUNT (100U)
#define SUBFRAME_Adcdata_LOOP_COUNT (1U)
#define SUBFRAME_Adcdata_COUNT (2U)
#define SUBFRAME_Adcdata_CHIRPIDX_EACH_BURST (1U)//(CHIRP_Adcdata_END_1_INDEX - CHIRP_Adcdata_START_0_INDEX +1U)
#define SUBFRAME_Adcdata_PERIODICITY_VAL (9200000U)
#define SUBFRAME_Adcdata_NUM_CHIRPS_TOTAL (SUBFRAME_Adcdata_CHIRPIDX_EACH_BURST * SUBFRAME_Adcdata_LOOP_COUNT)
#define PROFILE_ADC_SAMPLE_VAL (256U)


#endif /* SIGNALDESIGN_H_ */
