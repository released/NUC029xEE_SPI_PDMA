# NUC029xEE_SPI_PDMA
 NUC029xEE_SPI_PDMA


update @ 2022/08/05

1. initial SPI bus x 2 with PDMA interrupt enable

2. Enable define : ENABLE_AUTO_SS , to see different CS pin behavior about SPI waveform 

3. SPI1 auto send data per 1 sec , use terminal to send SPI0 data manually

4. press digit 1 , SPI0 waveform as below 

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_1_transmit_normal.jpg)	

press digit 2 (differnt SPI0 data) , SPI0 waveform as below 

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_2_transmit_getver.jpg)	

press digit 3 (differnt SPI0 data) , SPI0 waveform as below 

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_3_transmit_sendReceive.jpg)	

press digit 4 , will send SPI0 data , without PDMA 

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_4_transmit_nonPDMA.jpg)	

if Enable define : ENABLE_AUTO_SS , below is press digit 1 , SPI0 waveform as below 

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_4_transmit_nonPDMA.jpg)	

if Enable define : ENABLE_AUTO_SS , below is SPI1 data

![image](https://github.com/released/NUC029xEE_SPI_PDMA/blob/main/digit_4_transmit_nonPDMA.jpg)	


