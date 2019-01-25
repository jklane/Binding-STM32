/*
 * main.case
 *
 * Main source code for STM32 microprocessor.
 *
 * Author: Joe Lane
 * Created On: 01/24/2019
 * Updated On: 01/25/2019
 *
 */

#include "main.h"
#include "uart.h"

void LED_Init();
void Uart_Init();
void DMA_Init();
void Uart_putchar(char c);
void Uart_putstring(uint8_t *s, uint8_t len);

static UART_HandleTypeDef UartHandle;
static DMA_HandleTypeDef DMAHandle;

void DMA1_Stream6_IRQHandler()
{
  HAL_DMA_IRQHandler(&DMAHandle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
}


int main(void) {
  HAL_Init();
  LED_Init();
  Uart_Init();
  DMA_Init();

  uint8_t my_string[5];

  while (1)
  {
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    HAL_UART_Receive(&UartHandle, my_string, sizeof(my_string), 5000);
    Uart_putstring(&my_string[0], 5);
    //HAL_Delay(1000);
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

void DMA_Init()
{
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

  if (HAL_DMA_Init(&DMAHandle) != HAL_OK)
  {
    while(1){}
  }

  __HAL_LINKDMA(&UartHandle, hdmatx, DMAHandle);
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void Uart_putstring(uint8_t *s, uint8_t len)
{
  HAL_UART_Transmit(&UartHandle, s, len, 1000);
  /*
  for(int i = 0; i < len; i++)
  {
    Uart_putchar(s[i]);
  }
  */
}

void Uart_putchar(char c)
{
    HAL_UART_Transmit(&UartHandle, (uint8_t*)(&c), 1, 1000);
}

void SysTick_Handler(void) {
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}
