#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS_CLI.h"
#include "led-blackpill.hpp"

#define CLI_BUFFER_SIZE 128
#define CLI_OUTPUT_SIZE 512
#define CLI_QUEUE_SIZE  16

/* Definitions for blinkyTask */
osThreadId_t blinkyTaskHandle;
const osThreadAttr_t blinkyTask_attributes = {
  .name = "blinkyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
UART_HandleTypeDef huart1Handle;
uint8_t uartRxBuffer;

/* CLI task and queue handles */
osThreadId_t cliTaskHandle;
const osThreadAttr_t cliTask_attributes = {
  .name = "cliTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
osMessageQueueId_t cliQueueHandle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void UART_SendString(char *str);
void StartBlinkyTask(void *argument);
void CLITask(void *argument);

static BaseType_t taskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static const CLI_Command_Definition_t xTaskStats = {
  "task-stats",
  "\r\ntask-stats:\r\n Shows task statistics\r\n",
  taskStatsCommand,
  0
};

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

  FreeRTOS_CLIRegisterCommand(&xTaskStats);
  cliQueueHandle = osMessageQueueNew(CLI_QUEUE_SIZE, sizeof(uint8_t), NULL);

  /* Create the thread(s) */
  blinkyTaskHandle = osThreadNew(StartBlinkyTask, NULL, &blinkyTask_attributes);
  cliTaskHandle = osThreadNew(CLITask, NULL, &cliTask_attributes);

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
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
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

  /* Start UART receive interrupt */
  HAL_UART_Receive_IT(&huart1Handle, &uartRxBuffer, 1);
}

static void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart1Handle, (uint8_t*)str, strlen(str), 100);
}

/* UART interrupt callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    /* Send received byte to CLI queue */
	osMessageQueuePut(cliQueueHandle, &uartRxBuffer, 0, 0);

    /* Restart interrupt reception */
    HAL_UART_Receive_IT(&huart1Handle, &uartRxBuffer, 1);
  }
}

void StartBlinkyTask(void *argument)
{
  char *msg = "Hello World\r\n";
  HAL_UART_Transmit(&huart1Handle, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  int state = 0;
  Led *led = new Led_Stm32_BlackPill();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
    *led = (state ^= 1);
  }
}

static char cliInputBuffer[CLI_BUFFER_SIZE];
static char cliOutputBuffer[CLI_OUTPUT_SIZE];
static uint8_t cliInputIndex = 0;

void CLITask(void *argument)
{
  uint8_t receivedChar;
  BaseType_t xMoreDataToFollow;

  UART_SendString("\r\n\r\nFreeRTOS CLI Console\r\n");
  UART_SendString("Type 'help' for available commands\r\n");
  UART_SendString("> ");

  while (1) {
    if (osMessageQueueGet(cliQueueHandle, &receivedChar, NULL, osWaitForever) == osOK) {
      if (receivedChar == '\r' || receivedChar == '\n') {
        if (cliInputIndex > 0) {
          UART_SendString("\r\n");
          /* Null terminate input */
          cliInputBuffer[cliInputIndex] = '\0';

          /* Process command */
          do {
            xMoreDataToFollow = FreeRTOS_CLIProcessCommand(cliInputBuffer,
                                                           cliOutputBuffer,
                                                           CLI_OUTPUT_SIZE);
            /* Send output */
            UART_SendString(cliOutputBuffer);
          } while(xMoreDataToFollow != pdFALSE);
          /* Reset input buffer */
          cliInputIndex = 0;
        }
        /* Show prompt */
        UART_SendString("\r\n> ");
      } else if (receivedChar == '\b' || receivedChar == 127) { // Backspace
        if (cliInputIndex > 0) {
          cliInputIndex--;
          UART_SendString("\b \b");
        }
      } else if (cliInputIndex < (CLI_BUFFER_SIZE - 1)) {
        /* Echo character and add to buffer */
        cliInputBuffer[cliInputIndex++] = receivedChar;
        HAL_UART_Transmit(&huart1Handle, &receivedChar, 1, 100);
      }
    }
  }
}

static BaseType_t taskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
  strcpy(pcWriteBuffer, "Task\t\tState\tPrio\tStack\t#\r\n");
  strcat(pcWriteBuffer, "************************************************\r\n");

  #if (configUSE_TRACE_FACILITY == 1)
    vTaskList(pcWriteBuffer + strlen(pcWriteBuffer));
  #else
    strcat(pcWriteBuffer, "Task statistics not available - enable configUSE_TRACE_FACILITY\r\n");
  #endif

  return pdFALSE;
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
