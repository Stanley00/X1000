#ifndef _CLIENTPACKET_H
#define _CLIENTPACKET_H

unsigned long random_xid(void);
int send_discover(struct dhcpc_config_t *dhcpc_config,
        unsigned long xid, unsigned long requested);
int send_selecting(struct dhcpc_config_t *dhcpc_config,
        unsigned long xid, unsigned long server, unsigned long requested);
int send_renew(struct dhcpc_config_t *dhcpc_config,
        unsigned long xid, unsigned long server, unsigned long ciaddr);
int send_release(struct dhcpc_config_t *dhcpc_config,
        unsigned long server, unsigned long ciaddr);
int get_raw_packet(struct dhcpMessage *payload, int fd);

#endif
