/*
 * lwip_ethernetif.h
 *
 *  Created on: 2 dec. 2016
 *      Author: Marco
 */

#ifndef LWIP_ETHERNETIF_H
#define LWIP_ETHERNETIF_H

#include <lwip/opt.h>

#include "lwip/prot/etharp.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"







/**
 * @brief   Link poll interval.
 */
#if !defined(LWIP_LINK_POLL_INTERVAL) || defined(__DOXYGEN__)
#define LWIP_LINK_POLL_INTERVAL             S2ST(5)
#endif

/**
 * @brief   Transmission timeout.
 */
#if !defined(LWIP_SEND_TIMEOUT) || defined(__DOXYGEN__)
#define LWIP_SEND_TIMEOUT                   50
#endif

/**
 * @brief   Link speed.
 */
#if !defined(LWIP_LINK_SPEED) || defined(__DOXYGEN__)
#define LWIP_LINK_SPEED                     100000000
#endif







#if defined(LWIP_DEBUG)
#define LWIP_ETHERNET_THREAD_STACK_SIZE (3*128)
#else
#define LWIP_ETHERNET_THREAD_STACK_SIZE 256
#endif

#define LWIP_ETHERNET_THREAD_PRIORITY LOWPRIO

typedef enum {
  LWIP_ETH_UNINIT = 0,
  LWIP_ETH_STOP = 1,
  LWIP_ETH_READY = 2,
} lwip_ethernetif_state_t;

typedef struct LwIPEthernetIFDriver LwIPEthernetIFDriver;

typedef struct
{
  /**
   * @brief Pointer to the @p MACDriver associated to this driver.
   */
  MACDriver *mac_driver;
  /**
   * @brief Pointer to the @p MACConfig associated to this driver.
   */
  const MACConfig *mac_config;
  /**
   * @brief 2 character interface name.
   */
  char name[2];
  /**
   * @brief address
   */
  ip_addr_t address;
  /**
   * @brief netmask
   */
  ip_addr_t netmask;
  /**
   * @brief gateway
   */
  ip_addr_t gateway;
#if LWIP_DHCP
  /**
   * @brief use dhcp on this ethernet interface driver.
   */
  uint8_t use_dhcp;
#endif
} LwIPEthernetIFConfig;

struct LwIPEthernetIFDriver
{
  /**
   * @brief Driver state.
   */
  lwip_ethernetif_state_t state;
  /**
   * @brief configuration.
   */
  const LwIPEthernetIFConfig *config;
#if defined(_CHIBIOS_RT_)
  /**
   * @brief   Pointer to the thread.
   */
  thread_reference_t            tr;
  /**
   * @brief   Working area for the dedicated ethernet interface thread;
   */
  THD_WORKING_AREA(wa_ethernet, LWIP_ETHERNET_THREAD_STACK_SIZE);
#endif
  // lwip netif
  struct netif netif;
};

#ifdef __cplusplus
extern "C" {
#endif
  void lwipEthernetIFObjectInit(LwIPEthernetIFDriver *dp);
  void lwipEthernetIFStart(LwIPEthernetIFDriver *dp,
                           const LwIPEthernetIFConfig *config);
#ifdef __cplusplus
}
#endif

#endif /* LWIP_ETHERNETIF_H */
