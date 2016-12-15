# List of the required lwIP files.
LWIP = 	$(CHIBIOS)/ext/lwip

LWBINDSRC = \
        $(CHIBIOS)/os/various/lwip_bindings/lwipthread.c \
        $(CHIBIOS)/os/various/lwip_bindings/lwip_ethernetif.c \
        $(CHIBIOS)/os/various/lwip_bindings/lwip_ppposif.c \
        $(CHIBIOS)/os/various/lwip_bindings/arch/sys_arch.c

LWNETIFSRC = \
        $(LWIP)/src/netif/ethernet.c
        
LWPPPSRC= \
        $(LWIP)/src/netif/ppp/auth.c \
        $(LWIP)/src/netif/ppp/ccp.c \
        $(LWIP)/src/netif/ppp/chap-md5.c \
        $(LWIP)/src/netif/ppp/chap_ms.c \
        $(LWIP)/src/netif/ppp/chap-new.c \
        $(LWIP)/src/netif/ppp/demand.c \
        $(LWIP)/src/netif/ppp/eap.c \
        $(LWIP)/src/netif/ppp/ecp.c \
        $(LWIP)/src/netif/ppp/eui64.c \
        $(LWIP)/src/netif/ppp/fsm.c \
        $(LWIP)/src/netif/ppp/ipcp.c \
        $(LWIP)/src/netif/ppp/ipv6cp.c \
        $(LWIP)/src/netif/ppp/lcp.c \
        $(LWIP)/src/netif/ppp/magic.c \
        $(LWIP)/src/netif/ppp/mppe.c \
        $(LWIP)/src/netif/ppp/multilink.c \
        $(LWIP)/src/netif/ppp/ppp.c \
        $(LWIP)/src/netif/ppp/pppapi.c \
        $(LWIP)/src/netif/ppp/pppcrypt.c \
        $(LWIP)/src/netif/ppp/pppoe.c \
        $(LWIP)/src/netif/ppp/pppol2tp.c \
        $(LWIP)/src/netif/ppp/pppos.c \
        $(LWIP)/src/netif/ppp/upap.c \
        $(LWIP)/src/netif/ppp/utils.c \
        $(LWIP)/src/netif/ppp/vj.c \
        $(LWIP)/src/netif/ppp/polarssl/arc4.c \
        $(LWIP)/src/netif/ppp/polarssl/des.c \
        $(LWIP)/src/netif/ppp/polarssl/md4.c \
        $(LWIP)/src/netif/ppp/polarssl/md5.c \
        $(LWIP)/src/netif/ppp/polarssl/sha1.c

LWCORESRC = \
        $(LWIP)/src/core/def.c \
        $(LWIP)/src/core/dns.c \
        $(LWIP)/src/core/inet_chksum.c \
        $(LWIP)/src/core/init.c \
        $(LWIP)/src/core/ip.c \
        $(LWIP)/src/core/mem.c \
        $(LWIP)/src/core/memp.c \
        $(LWIP)/src/core/netif.c \
        $(LWIP)/src/core/pbuf.c \
        $(LWIP)/src/core/raw.c \
        $(LWIP)/src/core/stats.c \
        $(LWIP)/src/core/sys.c \
        $(LWIP)/src/core/tcp.c \
        $(LWIP)/src/core/tcp_in.c \
        $(LWIP)/src/core/tcp_out.c \
        $(LWIP)/src/core/timeouts.c \
        $(LWIP)/src/core/udp.c

LWIPV4SRC = \
        $(LWIP)/src/core/ipv4/autoip.c \
        $(LWIP)/src/core/ipv4/dhcp.c \
        $(LWIP)/src/core/ipv4/etharp.c \
        $(LWIP)/src/core/ipv4/icmp.c \
        $(LWIP)/src/core/ipv4/igmp.c \
        $(LWIP)/src/core/ipv4/ip4.c \
        $(LWIP)/src/core/ipv4/ip4_addr.c \
        $(LWIP)/src/core/ipv4/ip4_frag.c

LWIPV6SRC = \
        $(LWIP)/src/core/ipv6/dhcp6.c \
        $(LWIP)/src/core/ipv6/ethip6.c \
        $(LWIP)/src/core/ipv6/icmp6.c \
        $(LWIP)/src/core/ipv6/inet6.c \
        $(LWIP)/src/core/ipv6/ip6.c \
        $(LWIP)/src/core/ipv6/ip6_addr.c \
        $(LWIP)/src/core/ipv6/ip6_frag.c \
        $(LWIP)/src/core/ipv6/mld6.c \
        $(LWIP)/src/core/ipv6/nd6.c

LWAPISRC = \
        $(LWIP)/src/api/api_lib.c \
        $(LWIP)/src/api/api_msg.c \
        $(LWIP)/src/api/err.c \
        $(LWIP)/src/api/netbuf.c \
        $(LWIP)/src/api/netdb.c \
        $(LWIP)/src/api/netifapi.c \
        $(LWIP)/src/api/sockets.c \
        $(LWIP)/src/api/tcpip.c

LWSRC = \
        $(LWBINDSRC) \
        $(LWNETIFSRC) \
        $(LWCORESRC) \
        $(LWIPV4SRC) \
        $(LWIPV6SRC) \
        $(LWAPISRC) \
        $(LWPPPSRC)

LWINC = \
        $(CHIBIOS)/os/various/lwip_bindings \
        $(LWIP)/src/include
