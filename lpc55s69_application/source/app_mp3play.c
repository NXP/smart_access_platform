/*
 * Copyright (c) 2017 - 2018 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app.h"

#include "fsl_codec_common.h"
#include "fsl_wm8904.h"
#include "fsl_codec_adapter.h"

#include "mp3dec.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t *mp3_fd_buffer;
volatile uint32_t current_sample_rate = 0;
volatile uint8_t  mp3_decoder_flag = 0;
volatile int      outputSamps;
volatile uint32_t wav_read_index = 0;
volatile uint32_t wav_buf_size = 0;
volatile uint8_t *wav_buf0;
volatile uint8_t *wav_buf1;
volatile uint8_t  wav_play_sem = 0;

static dma_handle_t s_DmaTxHandle;
static i2s_config_t s_TxConfig;
static i2s_dma_handle_t s_TxHandle;
static i2s_transfer_t s_TxTransfer;

codec_handle_t codecHandle   = {0};
wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = 1, .codecI2CSourceClock = 12000000},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = 24576000,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
struct mp3_decoder
{
    /* mp3 information */
    HMP3Decoder  decoder;
    MP3FrameInfo frame_info;
    uint32_t  frames;

    /* mp3 read session */
    uint8_t  *read_buffer, *read_ptr;
    int32_t   read_offset;
    uint32_t  bytes_left, bytes_left_before_decoding;
    uint32_t  file_size;
    uint32_t  file_pos;
};


/*******************************************************************************
 * Variables
 ******************************************************************************/



/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief   NFC mS delay
 * @param   delay times / mS
 * @return  NULL
 */
volatile uint32_t s_delayCODECMsCnt1, s_delayCODECMsCnt2;
void CODEC_Delayms(uint32_t times)
{
    for (s_delayCODECMsCnt1 = 0; s_delayCODECMsCnt1 < times; s_delayCODECMsCnt1++)
    {
        for (s_delayCODECMsCnt2 = 0; s_delayCODECMsCnt2 < (11000); s_delayCODECMsCnt2++)
        {
            ;
        }
    }
}

volatile uint32_t s_delayCODECUsCnt1, s_delayCODECUsCnt2;
void CODEC_Delayus(uint32_t times)
{
    for (s_delayCODECUsCnt1 = 0; s_delayCODECUsCnt1 < times; s_delayCODECUsCnt1++)
    {
        for (s_delayCODECUsCnt2 = 0; s_delayCODECUsCnt2 < (10); s_delayCODECUsCnt2++)
        {
            ;
        }
    }
}

static void TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
#if 0
    /* Enqueue the same original buffer all over again */
    i2s_transfer_t *transfer = (i2s_transfer_t *)userData;
    I2S_TxTransferSendDMA(base, handle, *transfer);
#else

#endif
}

static void MP3TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    /* Enqueue the same original buffer all over again */
    if(wav_read_index == 0) {
        s_TxTransfer.data = &wav_buf0[0];
        s_TxTransfer.dataSize = wav_buf_size;
        wav_read_index = 1;
    }
    else {
        s_TxTransfer.data = &wav_buf1[0];
        s_TxTransfer.dataSize = wav_buf_size;
        wav_read_index = 0;
    }
    wav_play_sem = 1;
    I2S_TxTransferSendDMA(CODEC_I2S, &s_TxHandle, s_TxTransfer);
    I2S_TxTransferSendDMA(CODEC_I2S, &s_TxHandle, s_TxTransfer);
}

int streamer_pcm_mute(bool mute)
{
    status_t ret;
    ret = CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, mute);
    if (ret != kStatus_Success)
    {
        return 1;
    }
    return 0;
}

int streamer_pcm_set_volume(uint32_t volume)
{
    status_t ret;
    ret = CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, volume);
    if (ret != kStatus_Success)
    {
        return 1;
    }
    return 0;
}

void mp3_decoder_init(struct mp3_decoder* decoder)
{
	/* init read session */
	decoder->read_ptr = NULL;
	decoder->bytes_left_before_decoding = decoder->bytes_left = 0;
	decoder->frames = 0;

	decoder->read_buffer = &mp3_fd_buffer[0];
	if (decoder->read_buffer == NULL) return;

	MP3InitDecoder(&decoder->decoder);
}

void mp3_decoder_detach(struct mp3_decoder* decoder)
{
	/* release mp3 decoder */
    MP3FreeDecoder(decoder->decoder);
}

struct mp3_decoder* mp3_decoder_create()
{
    struct mp3_decoder* decoder;
	/* allocate object */
    decoder = (struct mp3_decoder*) malloc (sizeof(struct mp3_decoder));

    if (decoder == NULL)
    {
        PRINTF("@@@ Malloc Failed\r\n");
        return NULL;
    }
    memset(decoder, 0x00, sizeof(struct mp3_decoder));
    mp3_decoder_init(decoder);
    return decoder;
}

void mp3_decoder_delete(struct mp3_decoder* decoder)
{
	/* de-init mp3 decoder object */
	mp3_decoder_detach(decoder);
	/* release this object */
	free(decoder);
}


volatile uint32_t current_offset = 0;
static int32_t mp3_decoder_fill_buffer(struct mp3_decoder* decoder, uint8_t* buffer)
{
	uint32_t bytes_read;
	uint32_t bytes_to_read;

	if (decoder->bytes_left > 0) {
		// better: move unused rest of buffer to the start
		memmove(decoder->read_buffer, decoder->read_ptr, decoder->bytes_left);
	}

	bytes_to_read = (MP3_AUDIO_BUF_SZ - decoder->bytes_left) & ~(512 - 1);

    if(decoder->file_size <= decoder->file_pos)
    {
        PRINTF("@@@ Reach to size limitation\r\n");
        bytes_read = 0;
    }
    else
    {
        memcpy((uint8_t *)(decoder->read_buffer + decoder->bytes_left), &buffer[decoder->file_pos], bytes_to_read);
        decoder->file_pos += bytes_to_read;
        bytes_read = bytes_to_read;
    }

	if (bytes_read != 0)
	{
		decoder->read_ptr = decoder->read_buffer;
		decoder->read_offset = 0;
		decoder->bytes_left = decoder->bytes_left + bytes_read;
		return 0;
	}
	else
	{
		PRINTF("@@@ can't read more data\n");
		return -1;
	}
}

int mp3_decoder_run(struct mp3_decoder* decoder, uint8_t *buffer)
{
	int err;
	uint32_t  delta;

	if ((decoder->read_ptr == NULL) || decoder->bytes_left < 2*MAINBUF_SIZE)
	{
		if(mp3_decoder_fill_buffer(decoder, buffer) != 0)
			return false;
	}

	decoder->read_offset = MP3FindSyncWord(decoder->read_ptr, decoder->bytes_left);
	if (decoder->read_offset < 0)
	{
		/* discard this data */
		decoder->bytes_left = 0;
		return false;
	}

	decoder->read_ptr += decoder->read_offset;
	delta = decoder->read_offset;
	decoder->bytes_left -= decoder->read_offset;

	if (decoder->bytes_left < 1024)
	{
		/* fill more data */
		if(mp3_decoder_fill_buffer(decoder, buffer) != 0)
			return false;
	}

    /* get a decoder buffer */
	decoder->bytes_left_before_decoding = decoder->bytes_left;

    if(wav_read_index == 0) {

        err = MP3Decode(decoder->decoder, &decoder->read_ptr, (int*)&decoder->bytes_left, (short*)&wav_buf0[0], 0);
    }
    if(wav_read_index == 1) {
        err = MP3Decode(decoder->decoder, &decoder->read_ptr, (int*)&decoder->bytes_left, (short*)&wav_buf1[0], 0);
    }

	delta += (decoder->bytes_left_before_decoding - decoder->bytes_left);
	current_offset += delta;
	decoder->frames++;

	if (err != ERR_MP3_NONE) {
		switch (err) {
		case ERR_MP3_INDATA_UNDERFLOW:
			PRINTF("@@@ ERR_MP3_INDATA_UNDERFLOW\n");
			decoder->bytes_left = 0;
			if(mp3_decoder_fill_buffer(decoder, buffer) != 0) {
				/* release this memory block */
				PRINTF("@@@ mp3_decoder_fill_buffer != 0\r\n");
				return -1;
			}
			break;

		case ERR_MP3_MAINDATA_UNDERFLOW:
			/* do nothing - next call to decode will provide more mainData */
			PRINTF("@@@ ERR_MP3_MAINDATA_UNDERFLOW\n");
			break;

		default:
			PRINTF("@@@ unknown error: %d, left: %d\n", err, decoder->bytes_left);

			if (decoder->bytes_left > 0) {
				decoder->bytes_left --;
				decoder->read_ptr ++;
			}
			else {
				PRINTF("@@@ decorder errpr");
                return false;
			}
			break;
		}
	}
	else {
		/* no error */
		if(mp3_decoder_flag == 0) {
            MP3GetLastFrameInfo(decoder->decoder, &decoder->frame_info);
            /* set sample rate */
            if (decoder->frame_info.samprate != current_sample_rate){
                current_sample_rate = decoder->frame_info.samprate;
#if 0
                PRINTF("@@@ samprate %d\r\n", decoder->frame_info.samprate);
                PRINTF("@@@ bitrate  %d\r\n", decoder->frame_info.bitrate );
                PRINTF("@@@ nChans   %d\r\n", decoder->frame_info.nChans );
                PRINTF("@@@ version  %d\r\n", decoder->frame_info.version );
                PRINTF("@@@ bitsPerSample  %d\r\n", decoder->frame_info.bitsPerSample );
                PRINTF("@@@ outputSamps  %d\r\n", decoder->frame_info.outputSamps );
#endif
            }
            /*below is special handling for 19 and 20 mp3 files with mono channel and different sample rate*/
            if (decoder->frame_info.nChans == 1)
            {
                uint32_t cfg = CODEC_I2STX->CFG1;
                /* set mono mode */
                cfg |= I2S_CFG1_ONECHANNEL(true);
                CODEC_I2STX->CFG1 = cfg;

                /* set the clock divider */
                CODEC_I2STX->DIV = I2S_DIV_DIV((CODEC_I2S_CLKFREQ / 22050U / 32U / 1U) - 1UL);
            }
            else //decoder->frame_info.nChans == 2
            {
                uint32_t cfg = CODEC_I2STX->CFG1;
                /* set stereo mode */
                cfg &= ~(I2S_CFG1_ONECHANNEL(true));
                CODEC_I2STX->CFG1 = cfg;

                /* set the clock divider */
                CODEC_I2STX->DIV = I2S_DIV_DIV(I2S_CLOCK_DIVIDER - 1UL);
            }

            PRINTF("Setup playback of mp3 music\r\n");
            s_TxTransfer.data = &wav_buf0[0];
            s_TxTransfer.dataSize = 0;

            I2S_TxTransferCreateHandleDMA(CODEC_I2S, &s_TxHandle, &s_DmaTxHandle, MP3TxCallback, (void *)&s_TxTransfer);
            /* need to queue two transmit buffers so when the first one finishes transfer, the other immediatelly starts */
            wav_play_sem = 0;
            /* write to sound device */
            outputSamps = decoder->frame_info.outputSamps;
            mp3_decoder_flag = 1;
	    }
		if (outputSamps > 0){
            wav_buf_size = outputSamps * sizeof(uint16_t);

            if(mp3_decoder_flag == 1)
            {
                wav_play_sem = 0;
                mp3_decoder_flag = 2;
                s_TxTransfer.data = &wav_buf0[0];
                s_TxTransfer.dataSize = wav_buf_size;
                wav_read_index = 1;
                I2S_TxTransferSendDMA(CODEC_I2S, &s_TxHandle, s_TxTransfer);
            }
            while(wav_play_sem == 0)
            {
            }
            wav_play_sem = 0;
		}
	}
	return true;
}


void APP_MP3_Init(void)
{
    int ret;
    i2c_master_config_t masterConfig;

    PRINTF("@@@ MP3 Codec Init\r\n");

    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    IOCON->PIO[CODEC_PWR_PORT][CODEC_PWR_PIN]   = CODEC_PWR_FUNC;      /*  */
    IOCON->PIO[CODEC_MCLK_PORT][CODEC_MCLK_PIN] = CODEC_MCLK_FUNC;     /*  */
    IOCON->PIO[CODEC_SCK_PORT][CODEC_SCK_PIN]   = CODEC_SCK_FUNC;      /*  */
    IOCON->PIO[CODEC_WS_PORT][CODEC_WS_PIN]     = CODEC_WS_FUNC;       /*  */
    IOCON->PIO[CODEC_DAT_PORT][CODEC_DAT_PIN]   = CODEC_DAT_FUNC;      /*  */

#if CODEC_I2C_SHARED

#else
    IOCON->PIO[CODECSDA_PORT][CODECSDA_PIN] = (CODECSDA_FUNC  | IOCON_MODE_INACT | IOCON_INPFILT_ON | IOCON_DIGITAL_EN );  /* I2C SDA Pin */
    IOCON->PIO[CODECSCL_PORT][CODECSCL_PIN] = (CODECSCL_FUNC  | IOCON_MODE_INACT | IOCON_INPFILT_ON | IOCON_DIGITAL_EN );  /* I2C SCL Pin */
#endif

    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);


#if CODEC_I2C_SHARED

#else
    /* attach 12 MHz clock to FLEXCOMM4 (I2C master) */
    CLOCK_AttachClk(CODEC_I2C_CLKATTACH);
    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(CODEC_I2C_RST);
    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kI2C_2PinOpenDrain;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = CODEC_I2C_RATE;
    /* Initialize the I2C master peripheral */
    I2C_MasterInit(CODEC_I2C, &masterConfig, CODEC_I2C_CLKFREQ);
#endif

    SYSCTL_Init(SYSCTL);
    /* select signal source for share set */
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm6);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm6);
    /* select share set for special flexcomm signal */
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);

    /*!< Switch PLL0 clock source selector to XTAL16M */
    CLOCK_AttachClk(kFRO12M_to_PLL0);

    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(4U) | SYSCON_PLL0CTRL_SELP(3U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(3U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(6U),
        .pllsscg = {(SYSCON_PLL0SSCG0_MD_LBS(2447114240U)),(SYSCON_PLL0SSCG1_MD_MBS(0U) | (uint32_t)(kSS_MF_512) | (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC))},
        .pllRate = 24309895U,
        .flags   = PLL_SETUPFLAG_WAITLOCK};
    /*!< Configure PLL to the desired values */
    CLOCK_SetPLL0Freq(&pll0Setup);

    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 1U, false);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_AttachClk(kPLL0_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
    SYSCON->MCLKIO  = 1U;

    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);    /* reset FLEXCOMM for DMA0 */
    CLOCK_AttachClk(CODEC_I2S_CLKATTACH);           /* I2S clocks */
    RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);     /* reset FLEXCOMM for I2S */
    NVIC_ClearPendingIRQ(FLEXCOMM6_IRQn);           /* reset NVIC for FLEXCOMM6 and FLEXCOMM7 */
    EnableIRQ(FLEXCOMM6_IRQn);                      /* Enable interrupts for I2S */

    gpio_pin_config_t   gpioPinConfig;
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 0u;     /* output high as default. */
    GPIO_PinInit(GPIO, CODEC_PWR_PORT,  CODEC_PWR_PIN,  &gpioPinConfig);
    CODEC_Delayms(100);
    GPIO_PinWrite(GPIO, CODEC_PWR_PORT, CODEC_PWR_PIN, 1 );
    CODEC_Delayms(100);


    /*
     * masterSlave = kI2S_MasterSlaveNormalMaster;
     * mode = kI2S_ModeI2sClassic;
     * rightLow = false;
     * leftJust = false;
     * pdmData = false;
     * sckPol = false;
     * wsPol = false;
     * divider = 1;
     * oneChannel = false;
     * dataLength = 16;
     * frameLength = 32;
     * position = 0;
     * watermark = 4;
     * txEmptyZero = true;
     * pack48 = false;
     */
    I2S_TxGetDefaultConfig(&s_TxConfig);
    s_TxConfig.dataLength  = 16;
    s_TxConfig.frameLength = 32U;
    //s_TxConfig.oneChannel  = true;
    s_TxConfig.divider     = I2S_CLOCK_DIVIDER;
    s_TxConfig.masterSlave = kI2S_MasterSlaveNormalMaster;

    I2S_TxInit(CODEC_I2STX, &s_TxConfig);

    /* reset FLEXCOMM for DMA0 */
    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);
    DMA_Init(DMA0);
    DMA_EnableChannel(DMA0, CODEC_I2STX_CHANNEL);
    DMA_SetChannelPriority(DMA0, CODEC_I2STX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_DmaTxHandle, DMA0, CODEC_I2STX_CHANNEL);

    ret = CODEC_Init(&codecHandle, &boardCodecConfig);
    if (ret)
    {
        PRINTF("CODEC_Init failed\r\n");
    }
    else
    {
        PRINTF("CODEC_Init Success\r\n");
    }
    /* Invert the DAC data in order to output signal with correct polarity - set DACL_DATINV and DACR_DATINV = 1 */
    WM8904_WriteRegister((wm8904_handle_t *)codecHandle.codecDevHandle, WM8904_AUDIO_IF_0, 0x1850);
    /* Initial volume kept low for hearing safety. */
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 100);
}
#if 0
#include "music.h"


static void StartSoundPlayback(void)
{
    PRINTF("Setup looping playback of sine wave\r\n");

    s_TxTransfer.data     = &g_Music[0];
    s_TxTransfer.dataSize = sizeof(g_Music);

    I2S_TxTransferCreateHandleDMA(CODEC_I2STX, &s_TxHandle, &s_DmaTxHandle, TxCallback, (void *)&s_TxTransfer);
    /* need to queue two transmit buffers so when the first one
     * finishes transfer, the other immediatelly starts */
    I2S_TxTransferSendDMA(CODEC_I2STX, &s_TxHandle, s_TxTransfer);
    I2S_TxTransferSendDMA(CODEC_I2STX, &s_TxHandle, s_TxTransfer);
}
#endif

void audio_deinit(void)
{
    I2S_TransferAbortDMA(CODEC_I2STX, &s_TxHandle);
}

struct mp3_decoder* decoder;
uint8_t mp3_play(uint32_t filename)
{
#if 0
	StartSoundPlayback();
#else

    uint8_t *mp3_stream = NULL;
#if 1
    mp3_stream = 0x2002C000;
    memset(mp3_stream, 0x00, 0x5000);
#else
    mp3_stream = 0x20030000;
    memset(mp3_stream, 0x00, 0x4000);
#endif

    wav_buf0 = 0x20040000;
    memset((void *)wav_buf0, 0x00, 5120);

    wav_buf1 = 0x20041400;
    memset((void *)wav_buf1, 0x00, 5120);

    mp3_fd_buffer = 0x20042800;
    memset((void *)mp3_fd_buffer, 0x00, MP3_AUDIO_BUF_SZ);

    spiflash_read_file(filename*4, mp3_stream, 0x5000);

    PRINTF("S %x %x %x %x\r\n", mp3_stream[0], mp3_stream[1], mp3_stream[2], mp3_stream[3]);

    s_TxTransfer.data = &wav_buf0[0];
    s_TxTransfer.dataSize = 0;
    I2S_TxTransferCreateHandleDMA(CODEC_I2S, &s_TxHandle, &s_DmaTxHandle, MP3TxCallback, (void *)&s_TxTransfer);


	wav_play_sem = 0;
	wav_read_index = 0;
    mp3_decoder_flag = 0;
    wav_play_sem = 0;
    current_sample_rate = 0;
    PRINTF("@@@ MP3 start play\r\n");
    decoder = mp3_decoder_create();

    if (decoder != NULL)
    {
        decoder->file_size = 0x5000;
        decoder->file_pos  = 0;
        current_offset = 0;
        while (mp3_decoder_run(decoder, mp3_stream) != false);
        /* delete decoder object */
        mp3_decoder_delete(decoder);
    }
    else
    {
        PRINTF("@@@ Malloc Failed \r\n");
    }

    wav_buf0 = NULL;
    wav_buf1 = NULL;
    mp3_fd_buffer = NULL;
    audio_deinit();
    PRINTF("@@@ mp3 play end \r\n");

    return true;
#endif
}


uint8_t wav_play(uint32_t index)
{
    uint32_t i;
    uint8_t *wave_stream = NULL;
    uint32_t file_size;

#if 1
    wave_stream = 0x2002C000;
    memset(wave_stream, 0x00, 0x3C00);

    spiflash_read_file(index*4, wave_stream, 0x3C00);
#else
    wave_stream = 0x20030000;
    memset(wave_stream, 0x00, 0x3C00);

    spiflash_read_file(index*4, wave_stream, 0x3C00);
#endif
    wav_buf0 = 0x20040000;
    memset((void *)wav_buf0, 0x00, 5120);

    wav_buf1 = 0x20041400;
    memset((void *)wav_buf1, 0x00, 5120);

    s_TxTransfer.data = &wav_buf0[0];
    s_TxTransfer.dataSize = 0;
    I2S_TxTransferCreateHandleDMA(CODEC_I2S, &s_TxHandle, &s_DmaTxHandle, MP3TxCallback, (void *)&s_TxTransfer);

	wav_play_sem = 0;
	wav_read_index = 0;
    mp3_decoder_flag = 0;
    wav_play_sem = 0;
    current_sample_rate = 0;

    file_size =  (wave_stream[0x4F]<<24) | (wave_stream[0x4E]<<16) | (wave_stream[0x4D]<<8) | wave_stream[0x4C];
    current_offset = 0;
    I2S_TxTransferCreateHandleDMA(CODEC_I2S, &s_TxHandle, &s_DmaTxHandle, MP3TxCallback, (void *)&s_TxTransfer);
    wav_play_sem = 0;
    s_TxTransfer.data = &wav_buf0[0];
    s_TxTransfer.dataSize = 2048;
    I2S_TxTransferSendDMA(CODEC_I2S, &s_TxHandle, s_TxTransfer);
    while(current_offset < file_size)
    {
        if(wav_read_index == 0) {
            memcpy(wav_buf0, &wave_stream[current_offset+0x50], 2048);
            wav_buf_size = 2048;
            current_offset += 2048;
            wav_read_index = 1;
        }
        else {
            memcpy(wav_buf1, &wave_stream[current_offset+0x50], 2048);
            wav_buf_size = 2048;
            current_offset += 2048;
            wav_read_index = 0;
        }
        wav_play_sem = 0;
        while(wav_play_sem == 0);
    }

    wav_buf0      = NULL;
    wav_buf1      = NULL;

    audio_deinit();
    return true;
}

void APP_MP3_Play(uint32_t index)
{
#if 1
    /* Play MP3 file */
    if(index != 17)
    {
        mp3_play(index);
    }
    else
    {
        wav_play(index);
    }
#endif
}
