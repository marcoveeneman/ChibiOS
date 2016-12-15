/*
 * lwip_ppposif.h
 *
 *  Created on: 3 dec. 2016
 *      Author: Marco
 */

#ifndef LWIP_PPPOSIF_H
#define LWIP_PPPOSIF_H

#include <lwip/opt.h>
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"

#if defined(LWIP_DEBUG)
#define LWIP_PPPOS_THREAD_STACK_SIZE (5*128)
#else
#define LWIP_PPPOS_THREAD_STACK_SIZE 4*128
#endif

#define LWIP_PPPOS_THREAD_PRIORITY LOWPRIO

typedef enum {
  LWIP_PPPOS_UNINIT = 0,
  LWIP_PPPOS_STOP = 1,
  LWIP_PPPOS_READY = 2,
} lwip_ppposif_state_t;

typedef struct LwIPPPPoSIFDriver LwIPPPPoSIFDriver;

typedef struct
{
  SerialDriver *sd;

  const SerialConfig *serial_config;

  char name[2];
} LwIPPPPoSIFConfig;

struct LwIPPPPoSIFDriver
{
  lwip_ppposif_state_t state;

  const LwIPPPPoSIFConfig *config;
#if defined(_CHIBIOS_RT_)
  /**
   * @brief   Pointer to the thread.
   */
  thread_reference_t            tr;
  /**
   * @brief   Working area for the dedicated pppos interface thread;
   */
  THD_WORKING_AREA(wa_pppos, LWIP_PPPOS_THREAD_STACK_SIZE);
#endif
  struct netif netif;

  /**
   * @brief Flag to indicate .
   */
  bool restart;
};

#ifdef __cplusplus
extern "C" {
#endif
  void lwipPPPoSIFObjectInit(LwIPPPPoSIFDriver *dp);
  void lwipPPPoSIFStart(LwIPPPPoSIFDriver *dp,
                        const LwIPPPPoSIFConfig *config);
#ifdef __cplusplus
}
#endif

#endif /* LWIP_PPPOSIF_H */
