#ifndef WIFI_H
#define WIFI_H

void wifi_init(void);
int wifi_connect(char *ssid, char *psk);
void wifi_wait_for_ip_addr(void);

#endif // !WIFI_H
