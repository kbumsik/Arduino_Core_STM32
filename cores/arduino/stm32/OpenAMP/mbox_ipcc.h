/**
  ******************************************************************************
  * @file    mbox_ipcc.h
  * @author  MCD Application Team
  * @brief   Header for mbox_ipcc.c module
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

#ifndef MBOX_IPCC_H_
#define MBOX_IPCC_H_

#ifdef VIRTIOCON

/* Interrupt priority */
#ifndef IPCC_IRQ_PRIO
#define IPCC_IRQ_PRIO       1
#endif
#ifndef IPCC_IRQ_SUBPRIO
#define IPCC_IRQ_SUBPRIO    0
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int MAILBOX_Init(void);
int MAILBOX_Notify(void *priv, uint32_t id);
void MAILBOX_Notify_Rx_Buf_Free(void);

#endif /* VIRTIOCON */
#endif /* MBOX_IPCC_H_ */
