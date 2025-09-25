#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

#define LED_PIN  GPIO_PIN_12
#define LED_PORT GPIOB

/* Definitions for blinkyTask */
osThreadId_t blinkyTaskHandle;
const osThreadAttr_t blinkyTask_attributes = {
  .name = "blinkyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
UART_HandleTypeDef huart1Handle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
void StartBlinkyTask(void *argument);

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  /* Init scheduler */
  osKernelInitialize();

  /* Create the thread(s) */
  blinkyTaskHandle = osThreadNew(StartBlinkyTask, NULL, &blinkyTask_attributes);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

static void MX_USART1_UART_Init(void)
{
  huart1Handle.Instance = USART1;
  huart1Handle.Init.BaudRate = 115200;
  huart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
  huart1Handle.Init.StopBits = UART_STOPBITS_1;
  huart1Handle.Init.Parity = UART_PARITY_NONE;
  huart1Handle.Init.Mode = UART_MODE_TX_RX;
  huart1Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1Handle.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart1Handle);
}

void StartBlinkyTask(void *argument)
{
  char *msg = "Hello World\r\n";
  HAL_UART_Transmit(&huart1Handle, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  int state = 0;
  /* Infinite loop */
  for(;;)
  {
	char str[10];
    osDelay(1000);
	HAL_GPIO_WritePin(LED_PORT, LED_PIN, state ^= 1);
	snprintf(str, sizeof(str), "LED %s\r\n", state ? "Off" : "On");
	HAL_UART_Transmit(&huart1Handle, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
