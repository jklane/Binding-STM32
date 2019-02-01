/*
 * main.case
 *
 * Main source code for STM32 microprocessor.
 *
 * Author: Joe Lane
 * Created On: 01/24/2019
 * Updated On: 01/25/2019
 *
 * TODO
 *   - Check chip select
 *   - Probe SPI lines
 *   - Double check SPI configuration

 */

#include "main.h"
#include "uart.h"

void LED_Init();
void Uart_Init();
void DMA_Init();
void SPI_Init();
void Uart_putchar(char c);
void Uart_putstring(uint8_t *s, uint8_t len);

static UART_HandleTypeDef UartHandle;
static DMA_HandleTypeDef DMAHandle;
static SPI_HandleTypeDef spi = { .Instance = SPI1};



void DMA1_Stream6_IRQHandler() {
  HAL_DMA_IRQHandler(&DMAHandle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart) {
  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}

int main(void) {
  HAL_Init();
  LED_Init();
  Uart_Init();
  DMA_Init();
  SPI_Init();

  uint8_t my_string[5];
  uint8_t read_response;
  uint8_t write_addr;
  char str_response[5];

  while (1) {
      HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);

      // if nothing is pressed, effectively creates a 5 second delay
      //HAL_UART_Receive(&UartHandle, my_string, sizeof(my_string), 5000);
      HAL_Delay(1000);

      write_addr = 0x00; // who am I address
      HAL_SPI_TransmitReceive(&spi, (uint8_t *)&write_addr, (uint8_t *)&read_response, sizeof(write_addr), 5000);
      sprintf(str_response, "%d", read_response); // convert int to string

      //Uart_putstring(&my_string[0], 5);
      Uart_putstring((uint8_t *)&str_response[0], 5);
  }
}

void LED_Init() {
  LED_GPIO_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
}

void SPI_Init() {
  __SPI1_CLK_ENABLE();
  spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  spi.Init.Direction = SPI_DIRECTION_2LINES;
  spi.Init.CLKPhase = SPI_PHASE_2EDGE;
  spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
  spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  spi.Init.DataSize = SPI_DATASIZE_8BIT;
  spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi.Init.NSS = SPI_NSS_SOFT;
  spi.Init.TIMode = SPI_TIMODE_DISABLED;
  spi.Init.Mode = SPI_MODE_MASTER;

  if (HAL_SPI_Init(&spi) != HAL_OK) {
    asm("bkpt 255");
  }

}

void Uart_Init() {
  GPIO_InitTypeDef  GPIO_InitStruct;

  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();

  USARTx_CLK_ENABLE();

  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);



  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
  UartHandle.Instance          = USARTx;

  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

if(HAL_UART_Init(&UartHandle) != HAL_OK) {
  while(1){}
}

}

void DMA_Init() {
  __DMA1_CLK_ENABLE();
  DMAHandle.Instance = DMA1_Stream6;
  DMAHandle.Init.Channel = DMA_CHANNEL_4;

  DMAHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
  DMAHandle.Init.PeriphInc = DMA_PINC_DISABLE;
  DMAHandle.Init.MemInc = DMA_MINC_ENABLE;
  DMAHandle.Init.Mode = DMA_CIRCULAR;

  DMAHandle.Init.Priority = DMA_PRIORITY_VERY_HIGH;

  DMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  DMAHandle.Init.MemDataAlignment = DMA_PDATAALIGN_BYTE;

  if (HAL_DMA_Init(&DMAHandle) != HAL_OK) {
    while(1){}
  }

  __HAL_LINKDMA(&UartHandle, hdmatx, DMAHandle);
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void Uart_putstring(uint8_t *s, uint8_t len) {
  HAL_UART_Transmit(&UartHandle, s, len, 1000);
}

void Uart_putchar(char c) {
  HAL_UART_Transmit(&UartHandle, (uint8_t*)(&c), 1, 1000);
}

void SysTick_Handler(void) {
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}
