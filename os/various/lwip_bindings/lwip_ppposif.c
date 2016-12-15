/*
 * lwip_ppposif.c
 *
 *  Created on: 3 dec. 2016
 *      Author: Marco
 */

#include "hal.h"
#include "evtimer.h"

#include "lwip_ppposif.h"

#include "netif/ppp/pppos.h"
#include "lwip/netifapi.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define DISCONNECTED_ID           1

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static u32_t ppp_output_callback(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
  LwIPPPPoSIFDriver *dp = ctx;

  LWIP_UNUSED_ARG(pcb);

  return sdWrite(dp->config->sd, data, len);
}

static void ppp_link_status_callback(ppp_pcb *pcb, int err_code, void *ctx)
{
  LwIPPPPoSIFDriver *dp = ctx;

  LWIP_UNUSED_ARG(pcb);

  struct netif *pppif = pcb->netif;

  chprintf(&SD1, "ppp_link_status_callback %d\r\n", err_code);

  if (err_code == PPPERR_NONE) /* No error. */ {
    // initialization ok, let thread know.
    //dp->connected = true;

    chprintf(&SD1,"   our_ipaddr  = %s\n", ip4addr_ntoa(netif_ip4_addr(pppif)));
    chprintf(&SD1,"   his_ipaddr  = %s\n", ip4addr_ntoa(netif_ip4_gw(pppif)));
    chprintf(&SD1,"   netmask     = %s\n", ip4addr_ntoa(netif_ip4_netmask(pppif)));

  } else {
    // initialization failed? do something, retry or so? Check the win32 port.
    //dp->connected = false;
    chprintf(&SD1, "Sending restart...\r\n");
    dp->restart = true;
    // send event to thread to close the connection and try to re-establish a connection.
  }
}

/*
 * PPP input thread
 */
static THD_FUNCTION(ppposif_thread, p) {
  LwIPPPPoSIFDriver *dp = p;
  uint32_t len;
  uint8_t buffer[128];
  struct netif *netif;
  ppp_pcb *ppp;

  netif = &dp->netif;

  sdStart(dp->config->sd, dp->config->serial_config);

  //dp->restart = false;

  /* create pppos netif */
  chprintf(&SD1, "Creating ppp netif...\r\n");
  ppp = pppos_create(netif, ppp_output_callback, ppp_link_status_callback, dp);
  chprintf(&SD1, "Created netif.\r\n");

  while (true) {
    err_t err;

    dp->restart = false;

    chprintf(&SD1, "Connecting...\r\n");
    err = ppp_connect(ppp, 0);
    chprintf(&SD1, "Connecting..: %d\r\n", err);
    if (err != ERR_OK) {
      ppp_close(ppp, 1);
      continue;
    }

    chprintf(&SD1, "Connected.\r\n");

    while (dp->restart == false) {
      len = chnReadTimeout(dp->config->sd, buffer, sizeof(buffer),
                           OSAL_MS2ST(100)); // configurable time?
      if (len > 0) {
        pppos_input_tcpip(ppp, buffer, len);
      }
    }

    chprintf(&SD1, "Restarting...\r\n");
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Initializes an instance.
 *
 * @param[out] dp       pointer to the @p LwIPEthernetIFDriver object
 *
 * @init
 */
void lwipPPPoSIFObjectInit(LwIPPPPoSIFDriver *dp)
{
  dp->state = LWIP_PPPOS_STOP;
  dp->config = NULL;
  dp->restart = false;
#if defined(_CHIBIOS_RT_)
  dp->tr = NULL;
  /* Filling the thread working area here because the function
     @p chThdCreateI() does not do it.*/
#if CH_DBG_FILL_THREADS
  {
    void *wsp = dp->wa_pppos;
    _thread_memfill((uint8_t *)wsp,
                    (uint8_t *)wsp + sizeof (dp->wa_pppos),
                    CH_DBG_STACK_FILL_VALUE);
  }
#endif
#endif
}

/**
 * @brief   Configures and activates @p LwIPEthernetIFDriver driver.
 *
 * @param[in] dp        pointer to the @p LwIPEthernetIFDriver object
 * @param[in] config    pointer to the configuration
 *
 * @api
 */
void lwipPPPoSIFStart(LwIPPPPoSIFDriver *dp,
                      const LwIPPPPoSIFConfig *config)
{
  dp->config = config;

#if defined(_CHIBIOS_RT_)
  /* Creates the pppos thread. Note, it is created only once.*/
  if (dp->tr == NULL) {
    thread_descriptor_t pppos_descriptor = {
      "ppposif_thread",
      THD_WORKING_AREA_BASE(dp->wa_pppos),
      THD_WORKING_AREA_END(dp->wa_pppos),
      LWIP_PPPOS_THREAD_PRIORITY,
      ppposif_thread,
      (void *)dp
    };

    dp->tr = chThdCreate(&pppos_descriptor);
  }
#endif

  dp->state = LWIP_PPPOS_READY;
}
