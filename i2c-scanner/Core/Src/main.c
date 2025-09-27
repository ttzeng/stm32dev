#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

// Handle declarations
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

/* Definitions for blinkyTask */
osThreadId_t blinkyTaskHandle;
const osThreadAttr_t blinkyTask_attributes = {
  .name = "blinkyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for i2cScannerTask */
osThreadId_t i2cScannerTaskHandle;
const osThreadAttr_t i2cScannerTask_attributes = {
  .name = "i2cScannerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void StartBlinkyTask(void *argument);
void StartI2CScannerTask(void *argument);

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* Init scheduler */
  osKernelInitialize();

  /* Create the thread(s) */
  blinkyTaskHandle = osThreadNew(StartBlinkyTask, NULL, &blinkyTask_attributes);
  i2cScannerTaskHandle = osThreadNew(StartI2CScannerTask, NULL, &i2cScannerTask_attributes);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1) ;
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

void StartBlinkyTask(void *argument)
{
  int state = 0;
  /* Infinite loop */
  while (1) {
    char str[10];
    osDelay(500);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, state ^= 1);
  }
}

static void I2C_Scanner(void)
{
  char msg[64];
  uint8_t device_count = 0;
  HAL_StatusTypeDef result;

  sprintf(msg, "\r\nScanning I2C bus...\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  sprintf(msg, "     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  for (uint8_t row = 0; row < 8; row++) {
    sprintf(msg, "%02X: ", row * 16);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    for (uint8_t col = 0; col < 16; col++) {
      uint8_t address = (row * 16) + col;

      // Skip reserved addresses
      if (address < 0x08 || address > 0x77) {
        sprintf(msg, "   ");
      } else {
        // Try to communicate with device at this address
        result = HAL_I2C_IsDeviceReady(&hi2c1, address << 1, 1, 10);

        if (result == HAL_OK) {
          sprintf(msg, "%02X ", address);
          device_count++;
        } else {
          sprintf(msg, "-- ");
        }
      }
      HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }
    sprintf(msg, "\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  }

  if (device_count == 0) {
    sprintf(msg, "\r\nNo I2C devices found!\r\n");
  } else if (device_count == 1) {
    sprintf(msg, "\r\nFound 1 I2C device.\r\n");
  } else {
    sprintf(msg, "\r\nFound %d I2C devices.\r\n", device_count);
  }
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  sprintf(msg, "--------------------------------\r\n");
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
  * @brief  Function implementing the i2cScannerTask thread.
  * @param  argument: Not used
  * @retval None
  */
void StartI2CScannerTask(void *argument)
{
  // Send welcome message
  char welcome_msg[] = "\r\n=== STM32F103C8 I2C Scanner ===\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)welcome_msg, strlen(welcome_msg), HAL_MAX_DELAY);

  while (1) {
	I2C_Scanner();
    osDelay(5000);  // Scan every 5 seconds
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
