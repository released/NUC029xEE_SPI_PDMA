/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "project_config.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/
#define PLL_CLOCK   							72000000

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;
volatile uint32_t counter_systick = 0;

// #define ENABLE_AUTO_SS

#define SPI_FREQ 								(200000ul)

#define SPI0_MASTER_TX_DMA_CH   				(0)
#define SPI0_MASTER_RX_DMA_CH 					(1)
#define SPI0_MASTER_OPENED_CH_TX   				(1 << SPI0_MASTER_TX_DMA_CH)
#define SPI0_MASTER_OPENED_CH_RX 				(1 << SPI0_MASTER_RX_DMA_CH)

#define SPI1_MASTER_TX_DMA_CH   				(2)
#define SPI1_MASTER_RX_DMA_CH 					(3)
#define SPI1_MASTER_OPENED_CH_TX   				(1 << SPI1_MASTER_TX_DMA_CH)
#define SPI1_MASTER_OPENED_CH_RX 				(1 << SPI1_MASTER_RX_DMA_CH)

#if defined (ENABLE_AUTO_SS)
#define SPI0_SET_CS_LOW							((void) NULL)//(SPI_SET_SS_LOW(SPI0))//(PC0 = 0)
#define SPI0_SET_CS_HIGH						((void) NULL)//(SPI_SET_SS_HIGH(SPI0))//(PC0 = 1)
#define SPI1_SET_CS_LOW							((void) NULL)//(PC8 = 0)
#define SPI1_SET_CS_HIGH						((void) NULL)//(PC8 = 1)
#else
#define SPI0_SET_CS_LOW							(PC0 = 0)
#define SPI0_SET_CS_HIGH						(PC0 = 1)
#define SPI1_SET_CS_LOW							(PC8 = 0)
#define SPI1_SET_CS_HIGH						(PC8 = 1)
#endif


#define SPI_DATA_LEN 							(16)
uint8_t Spi0TxBuffer[SPI_DATA_LEN] = {0};
uint8_t Spi0RxBuffer[SPI_DATA_LEN] = {0};

uint8_t Spi1TxBuffer[SPI_DATA_LEN] = {0};
uint8_t Spi1RxBuffer[SPI_DATA_LEN] = {0};

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void systick_counter(void)
{
	counter_systick++;
}

uint32_t get_systick(void)
{
	return (counter_systick);
}

void set_systick(uint32_t t)
{
	counter_systick = t;
}

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    #if 1
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}
    #else
    if (memcmp(src, des, nBytes))
    {
        printf("\nMismatch!! - %d\n", nBytes);
        for (i = 0; i < nBytes; i++)
            printf("0x%02x    0x%02x\n", src[i], des[i]);
        return -1;
    }
    #endif

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}

void delay_ms(uint16_t ms)
{
	TIMER_Delay(TIMER0, 1000*ms);
}

void SPI_ClrBuffer(uint8_t idx)
{
    switch(idx)
    {
        case 0:
            reset_buffer(Spi0TxBuffer,0x00,SPI_DATA_LEN);        
            break;
        case 1:
            reset_buffer(Spi0RxBuffer,0x00,SPI_DATA_LEN);      
            break;

        case 2:
            reset_buffer(Spi1TxBuffer,0x00,SPI_DATA_LEN);        
            break;
        case 3:
            reset_buffer(Spi1TxBuffer,0x00,SPI_DATA_LEN);        
            break;
    }
}

void SPI_SET_CS_LOW(SPI_T *spi)
{
    if (spi == SPI0)
    {
        SPI0_SET_CS_LOW;
    }	
    else if (spi == SPI1)
    {
        SPI1_SET_CS_LOW;
    }
}

void SPI_SET_CS_HIGH(SPI_T *spi)
{
    if (spi == SPI0)
    {
        SPI0_SET_CS_HIGH;
    }	
    else if (spi == SPI1)
    {
        SPI1_SET_CS_HIGH;
    }
}

void SPI_transmit_finish(SPI_T *spi)
{
    while(!is_flag_set(flag_transmit_end));	
    while (SPI_IS_BUSY(spi));
    SPI_SET_CS_HIGH(spi);
}

void SPI_transmit_TxStart(SPI_T *spi , uint8_t* Buffer , uint8_t len)
{ 
    uint8_t i = 0;

    SPI_SET_CS_LOW(spi);

    //TX
    for (i = 0 ; i < len ; i++)
    {
        SPI_WRITE_TX(spi, Buffer[i]);
        SPI_TRIGGER(spi);	
        while (SPI_IS_BUSY(spi));
    }   
     
    SPI_SET_CS_HIGH(spi);
}

void SPI_PDMA_transmit_RxStart(SPI_T *spi , uint8_t* Buffer , uint8_t len)
{	
	set_flag(flag_transmit_end,DISABLE);

    if (spi == SPI0)
    {
        SPI_SET_CS_LOW(spi);
        
        SPI_ClrBuffer(SPI0_MASTER_RX_DMA_CH);

        //RX	
        PDMA_SetTransferCnt(SPI0_MASTER_RX_DMA_CH, PDMA_WIDTH_8, len);
        PDMA_SetTransferAddr(SPI0_MASTER_RX_DMA_CH, (uint32_t)&spi->RX, PDMA_SAR_FIX, (uint32_t)Buffer, PDMA_DAR_INC);		
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI0_MASTER_RX_DMA_CH, PDMA_SPI0_RX, FALSE, 0);

        PDMA_Trigger(SPI0_MASTER_RX_DMA_CH);

        SPI_TRIGGER_RX_PDMA(spi);

        PDMA_EnableInt(SPI0_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);	
    }	
    else if (spi == SPI1)
    {
        SPI_SET_CS_LOW(spi);
        
        SPI_ClrBuffer(SPI1_MASTER_RX_DMA_CH);

        //RX	
        PDMA_SetTransferCnt(SPI1_MASTER_RX_DMA_CH, PDMA_WIDTH_8, len);
        PDMA_SetTransferAddr(SPI1_MASTER_RX_DMA_CH, (uint32_t)&spi->RX, PDMA_SAR_FIX, (uint32_t)Buffer, PDMA_DAR_INC);		
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI1_MASTER_RX_DMA_CH, PDMA_SPI1_RX, FALSE, 0);

        PDMA_Trigger(SPI1_MASTER_RX_DMA_CH);

        SPI_TRIGGER_RX_PDMA(spi);

        PDMA_EnableInt(SPI1_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);
    }

    SPI_transmit_finish(spi);

    if (spi == SPI0)
    {
        PDMA_DisableInt(SPI0_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);
    }	
    else if (spi == SPI1)
    {
        PDMA_DisableInt(SPI1_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);
    }    
}

void SPI_PDMA_transmit_TxStart(SPI_T *spi , uint8_t* Buffer , uint8_t len)
{
	set_flag(flag_transmit_end,DISABLE);

    if (spi == SPI0)
    {
        SPI_SET_CS_LOW(spi);

        //TX
        PDMA_SetTransferCnt(SPI0_MASTER_TX_DMA_CH, PDMA_WIDTH_8, len);
        PDMA_SetTransferAddr(SPI0_MASTER_TX_DMA_CH, (uint32_t)Buffer, PDMA_SAR_INC, (uint32_t)&spi->TX, PDMA_DAR_FIX);		
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI0_MASTER_TX_DMA_CH, PDMA_SPI0_TX, FALSE, 0);

        PDMA_Trigger(SPI0_MASTER_TX_DMA_CH);
        
        SPI_TRIGGER_TX_PDMA(spi);

        PDMA_EnableInt(SPI0_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);	
    }	
    else if (spi == SPI1)
    {
        SPI_SET_CS_LOW(spi);

        //TX
        PDMA_SetTransferCnt(SPI1_MASTER_TX_DMA_CH, PDMA_WIDTH_8, len);
        PDMA_SetTransferAddr(SPI1_MASTER_TX_DMA_CH, (uint32_t)Buffer, PDMA_SAR_INC, (uint32_t)&spi->TX, PDMA_DAR_FIX);		
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI1_MASTER_TX_DMA_CH, PDMA_SPI1_TX, FALSE, 0);

        PDMA_Trigger(SPI1_MASTER_TX_DMA_CH);
        
        SPI_TRIGGER_TX_PDMA(spi);

        PDMA_EnableInt(SPI1_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);	
    }
    
    SPI_transmit_finish(spi);	

    if (spi == SPI0)
    {
        PDMA_DisableInt(SPI0_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);
    }	
    else if (spi == SPI1)
    {
        PDMA_DisableInt(SPI1_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);
    }
}

void PDMA_IRQHandler(void)
{
    #if 1
    /* Get PDMA Block transfer down interrupt status */
    if(PDMA_GET_CH_INT_STS(SPI0_MASTER_TX_DMA_CH) & PDMA_ISR_BLKD_IF_Msk)
    {
        /* Clear PDMA Block transfer down interrupt flag */   
        PDMA_CLR_CH_INT_FLAG(SPI0_MASTER_TX_DMA_CH, PDMA_ISR_BLKD_IF_Msk);   
        
        /* Handle PDMA block transfer done interrupt event */
        //insert process
        set_flag(flag_transmit_end,ENABLE);      
    }

    if(PDMA_GET_CH_INT_STS(SPI0_MASTER_RX_DMA_CH) & PDMA_ISR_BLKD_IF_Msk)
    {
        /* Clear PDMA Block transfer down interrupt flag */   
        PDMA_CLR_CH_INT_FLAG(SPI0_MASTER_RX_DMA_CH, PDMA_ISR_BLKD_IF_Msk);   
        
        /* Handle PDMA block transfer done interrupt event */
        //insert process
        set_flag(flag_transmit_end,ENABLE);      
    }      

    /* Get PDMA Block transfer down interrupt status */
    if(PDMA_GET_CH_INT_STS(SPI1_MASTER_TX_DMA_CH) & PDMA_ISR_BLKD_IF_Msk)
    {
        /* Clear PDMA Block transfer down interrupt flag */   
        PDMA_CLR_CH_INT_FLAG(SPI1_MASTER_TX_DMA_CH, PDMA_ISR_BLKD_IF_Msk);   
        
        /* Handle PDMA block transfer done interrupt event */
        //insert process
        set_flag(flag_transmit_end,ENABLE);      
    }

    if(PDMA_GET_CH_INT_STS(SPI1_MASTER_RX_DMA_CH) & PDMA_ISR_BLKD_IF_Msk)
    {
        /* Clear PDMA Block transfer down interrupt flag */   
        PDMA_CLR_CH_INT_FLAG(SPI1_MASTER_RX_DMA_CH, PDMA_ISR_BLKD_IF_Msk);   
        
        /* Handle PDMA block transfer done interrupt event */
        //insert process
        set_flag(flag_transmit_end,ENABLE);      
    }    

    #else
    uint32_t status = PDMA_GET_INT_STATUS();

    if(status & 0x1) {  /* CH0 */
        if(PDMA_GET_CH_INT_STS(0) & 0x2)
        {
  			//insert process
			set_flag(flag_transmit_end,ENABLE);          
        }
        PDMA_CLR_CH_INT_FLAG(0, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x2) {  /* CH1 */
        if(PDMA_GET_CH_INT_STS(1) & 0x2)
        {
			//insert process
			set_flag(flag_transmit_end,ENABLE);            
        }
        PDMA_CLR_CH_INT_FLAG(1, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x4) {  /* CH2 */
        if(PDMA_GET_CH_INT_STS(2) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(2, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x8) {  /* CH3 */
        if(PDMA_GET_CH_INT_STS(3) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(3, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x10) {  /* CH4 */
        if(PDMA_GET_CH_INT_STS(4) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(4, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x20) {  /* CH5 */
        if(PDMA_GET_CH_INT_STS(5) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(5, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x40) {  /* CH6 */
        if(PDMA_GET_CH_INT_STS(6) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(6, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x80) {  /* CH7 */
        if(PDMA_GET_CH_INT_STS(7) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(7, PDMA_ISR_BLKD_IF_Msk);
    } else if(status & 0x100) {  /* CH8 */
        if(PDMA_GET_CH_INT_STS(8) & 0x2)
        {
            
        }
        PDMA_CLR_CH_INT_FLAG(8, PDMA_ISR_BLKD_IF_Msk);
    } else
        printf("unknown interrupt !!\n");
    #endif
}

void SPI_PDMA_Init(SPI_T *spi)
{
    PDMA_T *pdma;

	set_flag(flag_transmit_end,DISABLE);

    SPI_SET_CS_LOW(spi);

    if (spi == SPI0)
    {
        /* Open PDMA Channel */
        PDMA_Open(SPI0_MASTER_OPENED_CH_TX | SPI0_MASTER_OPENED_CH_RX);

        //TX
        PDMA_SetTransferCnt(SPI0_MASTER_TX_DMA_CH, PDMA_WIDTH_8, SPI_DATA_LEN);
        /* Set source/destination address and attributes */
        PDMA_SetTransferAddr(SPI0_MASTER_TX_DMA_CH, (uint32_t)Spi0TxBuffer, PDMA_SAR_INC, (uint32_t)&spi->TX, PDMA_DAR_FIX);
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI0_MASTER_TX_DMA_CH, PDMA_SPI0_TX, FALSE, 0);

        /* Set Memory-to-Peripheral mode */
        pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * SPI0_MASTER_TX_DMA_CH));
        pdma->CSR = (pdma->CSR & (~PDMA_CSR_MODE_SEL_Msk)) | (0x2<<PDMA_CSR_MODE_SEL_Pos);

        SPI_TRIGGER_TX_PDMA(spi);	
        
        PDMA_EnableInt(SPI0_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);

        #if 1	
        //RX	
        PDMA_SetTransferCnt(SPI0_MASTER_RX_DMA_CH, PDMA_WIDTH_8, SPI_DATA_LEN);
        /* Set source/destination address and attributes */
        PDMA_SetTransferAddr(SPI0_MASTER_RX_DMA_CH, (uint32_t)&spi->RX, PDMA_SAR_FIX, (uint32_t)Spi0RxBuffer, PDMA_DAR_INC);
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI0_MASTER_RX_DMA_CH, PDMA_SPI0_RX, FALSE, 0);

        /* Set Memory-to-Peripheral mode */
        pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * SPI0_MASTER_RX_DMA_CH));
        pdma->CSR = (pdma->CSR & (~PDMA_CSR_MODE_SEL_Msk)) | (0x2<<PDMA_CSR_MODE_SEL_Pos);

        SPI_TRIGGER_RX_PDMA(spi);	

        PDMA_EnableInt(SPI0_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);	
        
        #endif

        NVIC_EnableIRQ(PDMA_IRQn);

        PDMA_Trigger(SPI0_MASTER_RX_DMA_CH);
        PDMA_Trigger(SPI0_MASTER_TX_DMA_CH);

    }	
    else if (spi == SPI1)
    {
        /* Open PDMA Channel */
        PDMA_Open(SPI1_MASTER_OPENED_CH_TX | SPI1_MASTER_OPENED_CH_RX);

        //TX
        PDMA_SetTransferCnt(SPI1_MASTER_TX_DMA_CH, PDMA_WIDTH_8, SPI_DATA_LEN);
        /* Set source/destination address and attributes */
        PDMA_SetTransferAddr(SPI1_MASTER_TX_DMA_CH, (uint32_t)Spi1TxBuffer, PDMA_SAR_INC, (uint32_t)&spi->TX, PDMA_DAR_FIX);
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI1_MASTER_TX_DMA_CH, PDMA_SPI1_TX, FALSE, 0);

        /* Set Memory-to-Peripheral mode */
        pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * SPI1_MASTER_TX_DMA_CH));
        pdma->CSR = (pdma->CSR & (~PDMA_CSR_MODE_SEL_Msk)) | (0x2<<PDMA_CSR_MODE_SEL_Pos);

        SPI_TRIGGER_TX_PDMA(spi);	
        
        PDMA_EnableInt(SPI1_MASTER_TX_DMA_CH, PDMA_IER_BLKD_IE_Msk);

        #if 1	
        //RX	
        PDMA_SetTransferCnt(SPI1_MASTER_RX_DMA_CH, PDMA_WIDTH_8, SPI_DATA_LEN);
        /* Set source/destination address and attributes */
        PDMA_SetTransferAddr(SPI1_MASTER_RX_DMA_CH, (uint32_t)&spi->RX, PDMA_SAR_FIX, (uint32_t)Spi1RxBuffer, PDMA_DAR_INC);
        /* Set request source; set basic mode. */
        PDMA_SetTransferMode(SPI1_MASTER_RX_DMA_CH, PDMA_SPI1_RX, FALSE, 0);

        /* Set Memory-to-Peripheral mode */
        pdma = (PDMA_T *)((uint32_t) PDMA0_BASE + (0x100 * SPI1_MASTER_RX_DMA_CH));
        pdma->CSR = (pdma->CSR & (~PDMA_CSR_MODE_SEL_Msk)) | (0x2<<PDMA_CSR_MODE_SEL_Pos);

        SPI_TRIGGER_RX_PDMA(spi);	

        PDMA_EnableInt(SPI1_MASTER_RX_DMA_CH, PDMA_IER_BLKD_IE_Msk);	
        
        #endif

        NVIC_EnableIRQ(PDMA_IRQn);

        PDMA_Trigger(SPI1_MASTER_RX_DMA_CH);
        PDMA_Trigger(SPI1_MASTER_TX_DMA_CH);
    }

	SPI_transmit_finish(spi);	
}

void SPI_Init(SPI_T *spi)
{
    if (spi == SPI0)
    {
        SPI_ClrBuffer(SPI0_MASTER_TX_DMA_CH);
        SPI_ClrBuffer(SPI0_MASTER_RX_DMA_CH);

        SPI_Open(spi, SPI_MASTER, SPI_MODE_0, 8, SPI_FREQ);

        #if defined (ENABLE_AUTO_SS)
        SPI_EnableAutoSS(spi, SPI_SS, SPI_SS_ACTIVE_LOW);
        #else
        SYS_UnlockReg();

        SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC0_Msk);
        SYS->GPC_MFP |= SYS_GPC_MFP_PC0_GPIO;
        SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC0_Msk);
        SYS->ALT_MFP |= SYS_ALT_MFP_PC0_GPIO;

        // USE SS : PC0 as manual control , 
        GPIO_SetMode(PC, BIT0, GPIO_PMD_OUTPUT);
        SYS_LockReg();

        SPI_DisableAutoSS(spi);

        #endif
    }	
    else if (spi == SPI1)
    {
        SPI_ClrBuffer(SPI1_MASTER_TX_DMA_CH);
        SPI_ClrBuffer(SPI1_MASTER_RX_DMA_CH);

        SPI_Open(spi, SPI_MASTER, SPI_MODE_0, 8, SPI_FREQ);

        #if defined (ENABLE_AUTO_SS)
        SPI_EnableAutoSS(spi, SPI_SS, SPI_SS_ACTIVE_LOW);
        #else
        SYS_UnlockReg();

        SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC8_Msk);
        SYS->GPC_MFP |= SYS_GPC_MFP_PC8_GPIO;
        SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC8_Msk);
        SYS->ALT_MFP |= SYS_ALT_MFP_PC8_GPIO;

        // USE SS : PC8 as manual control , 
        GPIO_SetMode(PC, BIT8, GPIO_PMD_OUTPUT);
        SYS_LockReg();

        SPI_DisableAutoSS(spi);

        #endif

    }

    SPI_SET_CS_HIGH(spi);
    SPI_PDMA_Init(spi);
}

void SPI1_process(void)
{
	uint16_t i = 0;	    
	static uint8_t data_cnt = 1;

    SPI_ClrBuffer(SPI1_MASTER_TX_DMA_CH);
    SPI_ClrBuffer(SPI1_MASTER_RX_DMA_CH);

    for (i = 0 ; i < SPI_DATA_LEN ; i++ )
    {
        Spi1TxBuffer[i] = 0x30 + i + data_cnt;
    }
    
    SPI_PDMA_transmit_TxStart(SPI1 , Spi1TxBuffer , SPI_DATA_LEN);    
	data_cnt = (data_cnt > 0x9) ? (1) : (data_cnt+1);    
}

void SPI0_process(void)
{
	uint16_t i = 0;	
	static uint8_t data_cnt = 1;
    uint8_t target_len = 0;

	if (is_flag_set(flag_transmit_sendReceive))
	{
		set_flag(flag_transmit_sendReceive , DISABLE);
        SPI_ClrBuffer(SPI0_MASTER_TX_DMA_CH);
        SPI_ClrBuffer(SPI0_MASTER_RX_DMA_CH);

		for (i = 0 ; i < SPI_DATA_LEN ; i++ )
		{
			Spi0TxBuffer[i] = 0x81 + i ;
		}
		Spi0TxBuffer[0] = 0x80;	
		Spi0TxBuffer[1] = 0x84;	
		Spi0TxBuffer[2] = 0x81;		
		Spi0TxBuffer[3] = 0x04;	
		
		Spi0TxBuffer[4] = 0x40 + data_cnt;	
		Spi0TxBuffer[5] = 0x51 + data_cnt;	
		Spi0TxBuffer[6] = 0x62 + data_cnt;	
		
		Spi0TxBuffer[SPI_DATA_LEN-3] = 0x90 + data_cnt;	
		Spi0TxBuffer[SPI_DATA_LEN-1] = 0x92 + data_cnt;
		
		SPI_PDMA_transmit_TxStart(SPI0 , Spi0TxBuffer , SPI_DATA_LEN);

		data_cnt = (data_cnt > 0x9) ? (1) : (data_cnt+1); 

		#if 0
		SPI_PDMA_transmit_RxStart(SPI0 , Spi0RxBuffer , SPI_DATA_LEN);
		printf("RX=======\r\n");
		dump_buffer_hex(Spi0RxBuffer,SPI_DATA_LEN);
		printf("\r\n");        
		#endif	
	}
	

	if (is_flag_set(flag_transmit_getver))
	{
		set_flag(flag_transmit_getver , DISABLE);
        SPI_ClrBuffer(SPI0_MASTER_TX_DMA_CH);
        SPI_ClrBuffer(SPI0_MASTER_RX_DMA_CH);        

		for (i = 0 ; i < SPI_DATA_LEN ; i++ )
		{
			Spi0TxBuffer[i] = 0x81 + i ;
		}
		Spi0TxBuffer[0] = 0x80;	
		Spi0TxBuffer[1] = 0x10 + data_cnt;	
		Spi0TxBuffer[2] = 0x21 + data_cnt;	
		Spi0TxBuffer[3] = 0x32 + data_cnt;			

		Spi0TxBuffer[SPI_DATA_LEN-4] = 'g';	
		Spi0TxBuffer[SPI_DATA_LEN-3] = 'e';	
		Spi0TxBuffer[SPI_DATA_LEN-2] = 't';	
		Spi0TxBuffer[SPI_DATA_LEN-1] = 0x41;	
		
		SPI_PDMA_transmit_TxStart(SPI0 , Spi0TxBuffer , SPI_DATA_LEN);

		data_cnt = (data_cnt > 0x9) ? (1) : (data_cnt+1); 

		#if 0
		SPI_PDMA_transmit_RxStart(SPI0 , Spi0RxBuffer , SPI_DATA_LEN);
		printf("RX=======\r\n");
		dump_buffer_hex(Spi0RxBuffer,SPI_DATA_LEN);
		printf("\r\n");        
		#endif
		
	}
	

	if (is_flag_set(flag_transmit_normal))
	{
		set_flag(flag_transmit_normal , DISABLE);
        SPI_ClrBuffer(SPI0_MASTER_TX_DMA_CH);
        SPI_ClrBuffer(SPI0_MASTER_RX_DMA_CH);        

		for (i = 0 ; i < SPI_DATA_LEN ; i++ )
		{
			Spi0TxBuffer[i] = 0x80 + i ;
		}

		Spi0TxBuffer[SPI_DATA_LEN-2] = 0x90 + data_cnt;	
		Spi0TxBuffer[SPI_DATA_LEN-1] = 0x92 + data_cnt;
		
		SPI_PDMA_transmit_TxStart(SPI0 , Spi0TxBuffer , SPI_DATA_LEN);

		data_cnt = (data_cnt > 0x9) ? (1) : (data_cnt+1); 

		#if 0
		SPI_PDMA_transmit_RxStart(SPI0 , Spi0RxBuffer , SPI_DATA_LEN);
		printf("RX=======\r\n");
		dump_buffer_hex(Spi0RxBuffer,SPI_DATA_LEN);
		printf("\r\n");        
		#endif		
	}

	if (is_flag_set(flag_transmit_nonPDMA))
	{
		set_flag(flag_transmit_nonPDMA , DISABLE);
        SPI_ClrBuffer(SPI0_MASTER_TX_DMA_CH);

        target_len = 8;
  
		for (i = 0 ; i < target_len ; i++ )
		{
			Spi0TxBuffer[i] = 0x40 + i ;
		}

		Spi0TxBuffer[target_len-2] = 0xAA + data_cnt;	
		Spi0TxBuffer[target_len-1] = 0xCC + data_cnt;
		
		SPI_transmit_TxStart(SPI0 , Spi0TxBuffer , target_len);

		data_cnt = (data_cnt > 0x9) ? (1) : (data_cnt+1); 
    }
}

void GPIO_Init (void)
{
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB4_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB4_GPIO);
	
    GPIO_SetMode(PB, BIT4, GPIO_PMD_OUTPUT);

}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned long delay)
{  
    
    uint32_t tickstart = get_systick(); 
    uint32_t wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void loop(void)
{
	// static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (is_flag_set(flag_timer_period_1000ms))
    {
        set_flag(flag_timer_period_1000ms ,DISABLE);

        // printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB4 ^= 1;   
        SPI1_process();     
    }

    SPI0_process();
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);
    printf("digit : %c\r\n" ,res);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
                set_flag(flag_transmit_normal , ENABLE);
				break;

			case '2':
                set_flag(flag_transmit_getver , ENABLE);
				break;

			case '3':
                set_flag(flag_transmit_sendReceive , ENABLE);
				break;

            case '4' :            
                set_flag(flag_transmit_nonPDMA , ENABLE);            
                break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART02_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_ISR_RDA_INT_Msk | UART_ISR_TOUT_IF_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
        }
    }

}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_IER_RDA_IEN_Msk | UART_IER_TOUT_IEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetPCLKFreq : %8d\r\n",CLK_GetPCLKFreq());	
	#endif	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    // CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    // CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));
	
    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1_S_HIRC, 0);

    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL1_SPI0_S_HCLK, MODULE_NoMsk);
    CLK_EnableModuleClock(SPI0_MODULE);

    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL1_SPI1_S_HCLK, MODULE_NoMsk);
    CLK_EnableModuleClock(SPI1_MODULE);

    CLK_EnableModuleClock(PDMA_MODULE);

    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);

    SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC0_Msk | SYS_GPC_MFP_PC1_Msk | SYS_GPC_MFP_PC2_Msk | SYS_GPC_MFP_PC3_Msk);
    SYS->GPC_MFP |= SYS_GPC_MFP_PC0_SPI0_SS0 | SYS_GPC_MFP_PC1_SPI0_CLK | SYS_GPC_MFP_PC2_SPI0_MISO0 | SYS_GPC_MFP_PC3_SPI0_MOSI0;
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC0_Msk | SYS_ALT_MFP_PC1_Msk | SYS_ALT_MFP_PC2_Msk | SYS_ALT_MFP_PC3_Msk);
    SYS->ALT_MFP |= SYS_ALT_MFP_PC0_SPI0_SS0 | SYS_ALT_MFP_PC1_SPI0_CLK | SYS_ALT_MFP_PC2_SPI0_MISO0 | SYS_ALT_MFP_PC3_SPI0_MOSI0;

    SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC8_Msk | SYS_GPC_MFP_PC9_Msk | SYS_GPC_MFP_PC10_Msk | SYS_GPC_MFP_PC11_Msk);
    SYS->GPC_MFP |= SYS_GPC_MFP_PC8_SPI1_SS0 | SYS_GPC_MFP_PC9_SPI1_CLK | SYS_GPC_MFP_PC10_SPI1_MISO0 | SYS_GPC_MFP_PC11_SPI1_MOSI0;
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PC8_Msk);
    SYS->ALT_MFP |= SYS_ALT_MFP_PC8_SPI1_SS0;


   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
    SYS_Init();

	GPIO_Init();
	UART0_Init();
	TIMER1_Init();

    SysTick_enable(1000);

    SPI_Init(SPI0);
    SPI_Init(SPI1);

    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
