/*
 * ethernetif.c
 *
 *  Created on: 2 dec. 2016
 *      Author: Marco
 */


#include "hal.h"
#include "evtimer.h"

#include "lwip_ethernetif.h"

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"

#include "lwip/tcpip.h"

#include "lwip/netifapi.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define PERIODIC_TIMER_ID       1
#define FRAME_RECEIVED_ID       2

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*
 * Initialization.
 */
static void low_level_init(struct netif *netif) {
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an Ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* Do whatever else is needed to initialize interface. */
}

/*
 * Transmits a frame.
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
  struct pbuf *q;
  MACTransmitDescriptor td;
  LwIPEthernetIFDriver *dp;

  dp = netif->state;

  (void)netif;
  if (macWaitTransmitDescriptor(dp->config->mac_driver, &td, MS2ST(LWIP_SEND_TIMEOUT)) != MSG_OK)
    return ERR_TIMEOUT;

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE);        /* drop the padding word */
#endif

  /* Iterates through the pbuf chain. */
  for(q = p; q != NULL; q = q->next)
    macWriteTransmitDescriptor(&td, (uint8_t *)q->payload, (size_t)q->len);
  macReleaseTransmitDescriptor(&td);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE);         /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

/*
 * Receives a frame.
 */
static struct pbuf *low_level_input(struct netif *netif) {
  MACReceiveDescriptor rd;
  struct pbuf *p, *q;
  u16_t len;
  LwIPEthernetIFDriver *dp = netif->state;

  (void)netif;
  if (macWaitReceiveDescriptor(dp->config->mac_driver, &rd, TIME_IMMEDIATE) == MSG_OK) {
    len = (u16_t)rd.size;

#if ETH_PAD_SIZE
    len += ETH_PAD_SIZE;        /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL) {

#if ETH_PAD_SIZE
      pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

      /* Iterates through the pbuf chain. */
      for(q = p; q != NULL; q = q->next)
        macReadReceiveDescriptor(&rd, (uint8_t *)q->payload, (size_t)q->len);
      macReleaseReceiveDescriptor(&rd);

#if ETH_PAD_SIZE
      pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

      LINK_STATS_INC(link.recv);
    }
    else {
      macReleaseReceiveDescriptor(&rd);
      LINK_STATS_INC(link.memerr);
      LINK_STATS_INC(link.drop);
    }
    return p;
  }
  return NULL;
}

/*
 * Initialization.
 */
static err_t ethernetif_init(struct netif *netif) {
  LwIPEthernetIFDriver *dp = netif->state;

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LWIP_LINK_SPEED);

//  netif->state = NULL;
  netif->name[0] = dp->config->name[0];
  netif->name[1] = dp->config->name[1];
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

/**
 * @brief LWIP ethernet interface handling thread.
 *
 * @param[in] p         pointer to a @p LwIPEthernetIFDriver structure.
 */
static THD_FUNCTION(ethernetif_thread, p) {
  LwIPEthernetIFDriver *dp;
  struct netif *netif;
  event_timer_t evt;
  event_listener_t el0, el1;
  const ip_addr_t *ip, *gateway, *netmask;
  unsigned i;
  MACDriver *md;

  dp = (LwIPEthernetIFDriver *)p;

  md = dp->config->mac_driver;
  netif = &dp->netif;

  for (i = 0; i < ETHARP_HWADDR_LEN; i++)
    netif->hwaddr[i] = dp->config->mac_config->mac_address[i];

  ip = &dp->config->address;
  gateway = &dp->config->gateway;
  netmask = &dp->config->netmask;

  macStart(md, dp->config->mac_config);
  netifapi_netif_add(netif, ip, netmask, gateway, dp, ethernetif_init, tcpip_input);

  netifapi_netif_set_default(netif);
  netifapi_netif_set_up(netif);

  /* Setup event sources.*/
  evtObjectInit(&evt, LWIP_LINK_POLL_INTERVAL);
  evtStart(&evt);
  chEvtRegisterMask(&evt.et_es, &el0, PERIODIC_TIMER_ID);
  chEvtRegisterMask(macGetReceiveEventSource(md), &el1, FRAME_RECEIVED_ID);
  chEvtAddEvents(PERIODIC_TIMER_ID | FRAME_RECEIVED_ID);

  while (true) {
    eventmask_t mask = chEvtWaitAny(ALL_EVENTS);
    if (mask & PERIODIC_TIMER_ID) {
      bool current_link_status = macPollLinkStatus(md);
      if (current_link_status != netif_is_link_up(netif)) {
        if (current_link_status) {
          tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_up,
                                     netif, 0);
#if LWIP_DHCP
          if (dp->config->use_dhcp)
            netifapi_dhcp_start(netif);
#endif
        }
        else {
          tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_down,
                                     netif, 0);
#if LWIP_DHCP
          if (dp->config->use_dhcp)
            netifapi_dhcp_stop(netif);
#endif
        }
      }
    }
    if (mask & FRAME_RECEIVED_ID) {
      struct pbuf *p;

      /* move received packet into a new pbuf */
      p = low_level_input(netif);
      /* if no packet could be read, silently ignore this */
      if (p != NULL) {
        /* pass all packets to tcpip_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK) {
          LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
          pbuf_free(p);
          p = NULL;
        }
      }
    }
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
void lwipEthernetIFObjectInit(LwIPEthernetIFDriver *dp)
{
  dp->state = LWIP_ETH_STOP;
  dp->config = NULL;
#if defined(_CHIBIOS_RT_)
  dp->tr = NULL;
  /* Filling the thread working area here because the function
     @p chThdCreateI() does not do it.*/
#if CH_DBG_FILL_THREADS
  {
    void *wsp = dp->wa_ethernet;
    _thread_memfill((uint8_t *)wsp,
                    (uint8_t *)wsp + sizeof (dp->wa_ethernet),
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
void lwipEthernetIFStart(LwIPEthernetIFDriver *dp,
                         const LwIPEthernetIFConfig *config)
{
  dp->config = config;

#if defined(_CHIBIOS_RT_)
  /* Creates the ethernet thread. Note, it is created only once.*/
  if (dp->tr == NULL) {
    thread_descriptor_t ethernet_descriptor = {
      "ethernetif_thread",
      THD_WORKING_AREA_BASE(dp->wa_ethernet),
      THD_WORKING_AREA_END(dp->wa_ethernet),
      LWIP_ETHERNET_THREAD_PRIORITY,
      ethernetif_thread,
      (void *)dp
    };

    dp->tr = chThdCreate(&ethernet_descriptor);
  }
#endif

  dp->state = LWIP_ETH_READY;
}
