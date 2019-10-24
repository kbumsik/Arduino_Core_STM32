/**
  ******************************************************************************
  * @file    mbox_ipcc.c
  * @author  MCD Application Team
  * @brief   This file provides code for the configuration
  *                      of the mailbox_ipcc_if.c MiddleWare.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifdef VIRTIOCON

/*
 * Channel direction and usage:
 *
 *  ========   <-- new msg ---=============--------<------   =======
 * ||      ||                || CHANNEL 1 ||                ||     ||
 * ||  A7  ||  ------->-------=============--- buf free-->  || M4  ||
 * ||      ||                                               ||     ||
 * ||master||  <-- buf free---=============--------<------  ||slave||
 * ||      ||                || CHANNEL 2 ||                ||     ||
 *  ========   ------->-------=============----new msg -->   =======
 */

/* Includes ------------------------------------------------------------------*/
#include "openamp/open_amp.h"
#include "stm32_def.h"
#include "openamp_conf.h"

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* Private define ------------------------------------------------------------*/
#define MASTER_CPU_ID       0
#define REMOTE_CPU_ID       1
#define IPCC_CPU_A7         MASTER_CPU_ID
#define IPCC_CPU_M4         REMOTE_CPU_ID

#define RX_NO_MSG           0
#define RX_NEW_MSG          1
#define RX_BUF_FREE         2

/* Private variables ---------------------------------------------------------*/
IPCC_HandleTypeDef hipcc;
extern struct rpmsg_virtio_device rvdev;




/* Private function prototypes -----------------------------------------------*/
void IPCC_channel1_callback(IPCC_HandleTypeDef *hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir);
void IPCC_channel2_callback(IPCC_HandleTypeDef *hipcc, uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir);

/**
  * @brief  Initialize MAILBOX with IPCC peripheral
  * @param  None
  * @retval : Operation result
  */
int MAILBOX_Init(void)
{
  __HAL_RCC_IPCC_CLK_ENABLE();
  HAL_NVIC_SetPriority(IPCC_RX1_IRQn, IPCC_IRQ_PRIO, IPCC_IRQ_SUBPRIO);
  HAL_NVIC_EnableIRQ(IPCC_RX1_IRQn);
  hipcc.Instance = IPCC;
  if (HAL_IPCC_Init(&hipcc) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_IPCC_ActivateNotification(&hipcc, IPCC_CHANNEL_1, IPCC_CHANNEL_DIR_RX,
                                    IPCC_channel1_callback) != HAL_OK) {
    OPENAMP_log_err("%s: ch_1 RX fail\n", __func__);
    return -1;
  }

  if (HAL_IPCC_ActivateNotification(&hipcc, IPCC_CHANNEL_2, IPCC_CHANNEL_DIR_RX,
                                    IPCC_channel2_callback) != HAL_OK) {
    OPENAMP_log_err("%s: ch_2 RX fail\n", __func__);
    return -1;
  }

  return 0;
}

/**
  * @brief  Callback function called by OpenAMP MW to notify message processing
  * @param  VRING id
  * @retval Operation result
  */
int MAILBOX_Notify(void *priv, uint32_t id)
{
  uint32_t channel;
  (void)priv;

  /* Called after virtqueue processing: time to inform the remote */
  if (id == VRING0_ID) {
    channel = IPCC_CHANNEL_1;
    OPENAMP_log_dbg("Send msg on ch_1\r\n");
  } else if (id == VRING1_ID) {
    /* Note: the OpenAMP framework never notifies this */
    channel = IPCC_CHANNEL_2;
    OPENAMP_log_dbg("Send 'buff free' on ch_2\r\n");
    return -1;
  } else {
    OPENAMP_log_err("invalid vring (%d)\r\n", (int)id);
    return -1;
  }

  /* Check that the channel is free (otherwise wait until it is) */
  if (HAL_IPCC_GetChannelStatus(&hipcc, channel, IPCC_CHANNEL_DIR_TX) == IPCC_CHANNEL_STATUS_OCCUPIED) {
    OPENAMP_log_dbg("Waiting for channel to be freed\r\n");
    while (HAL_IPCC_GetChannelStatus(&hipcc, channel, IPCC_CHANNEL_DIR_TX) == IPCC_CHANNEL_STATUS_OCCUPIED)
      ;
  }

  /* Inform A7 (either new message, or buf free) */
  HAL_IPCC_NotifyCPU(&hipcc, channel, IPCC_CHANNEL_DIR_TX);

  return 0;
}

/**
  * @brief  Notify Rx buffer is free to Master
  */
void MAILBOX_Notify_Rx_Buf_Free()
{
  HAL_IPCC_NotifyCPU(&hipcc, IPCC_CHANNEL_2, IPCC_CHANNEL_DIR_RX);
}

/* Private function  ---------------------------------------------------------*/
/* Callback from IPCC Interrupt Handler: Master Processor informs that there are some free buffers */
void IPCC_channel1_callback(IPCC_HandleTypeDef *hipcc,
                            uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{
  /* Inform A7 that we have received the 'buff free' msg */
  OPENAMP_log_dbg("Ack 'buff free' message on ch1\r\n");
  HAL_IPCC_NotifyCPU(hipcc, ChannelIndex, IPCC_CHANNEL_DIR_RX);
  rproc_virtio_notified(rvdev.vdev, VRING0_ID);
}

/* Callback from IPCC Interrupt Handler: new message received from Master Processor */
void IPCC_channel2_callback(IPCC_HandleTypeDef *hipcc,
                            uint32_t ChannelIndex, IPCC_CHANNELDirTypeDef ChannelDir)
{
  (void) hipcc;
  (void) ChannelIndex;
  (void) ChannelDir;
  /* Don't inform A7 here, do it when the buffer has more than RPMSG_BUFFER_SIZE.
   * See MAILBOX_Notify_Rx_Buf_Free() and VirIOSerial.cpp.
   */
  rproc_virtio_notified(rvdev.vdev, VRING1_ID);
  /* The OpenAMP framework does not notify for free buf: do it here */
  rproc_virtio_notified(NULL, VRING1_ID);
}

/**
  * @brief This function handles IPCC RX1 occupied interrupt.
  */
void IPCC_RX1_IRQHandler(void)
{
  /* USER CODE BEGIN IPCC_RX1_IRQn 0 */
  log_dbg("%s: IT RX1\r\n", __func__);
  /* USER CODE END IPCC_RX1_IRQn 0 */
  HAL_IPCC_RX_IRQHandler(&hipcc);
  /* USER CODE BEGIN IPCC_RX1_IRQn 1 */

  /* USER CODE END IPCC_RX1_IRQn 1 */
}

#endif /* VIRTIOCON */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
