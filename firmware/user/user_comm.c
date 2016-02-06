
#include "platform.h"
#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "user_interface.h"

#include "user_display.h"
#include "user_comm.h"

#include "ets_sys.h"
#include "driver/uart.h"
#include "mem.h"

#include "espconn.h"
#include "lwip/udp.h"









// wifi connection variables
const char* ssid = "possan";
const char* password = "kebabkebab";
bool wifiConnected = false;
const unsigned int localPort = 3333;
const char *remoteIP = "192.168.1.107"; 
bool udpConnected = false;
long totalbytes = 0;
long totalpackets = 0;
struct udp_pcb *ptel_pcb;
static esp_tcp global_udp;                                  // udp connect var (see espconn.h)
static struct espconn *last_conn;
uint8_t *last_message;
uint8_t last_message_len;






int parse_osc(unsigned char *bufptr, int siz) {
  int i;
  if (memcmp(bufptr, "/cube1/led\0\0", 12) == 0 && siz == 24) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/led command!");
    #endif

    //
    // 0000   2f 63 75 62 65 31 2f 6c 65 64 00 00 2c 69 69 00  /cube1/led..,ii.
    // 0010   00 00 00 06 00 00 00 01                          ........
    //

    if (bufptr[12] == 0x2C && bufptr[13] == 'i' && bufptr[14] == 'i') {
      // Serial.println("Has correct argument signature");

      unsigned int1value = bufptr[19];
      unsigned int2value = bufptr[23];
      // Serial.print("Led ");
      // Serial.print(int1value);
      // Serial.print(" = ");
      // Serial.print(int2value);
      // Serial.print("\n");

      // if (int1value < NUMKEYS) {
      // bits[int1value] = int2value > 0;
      display_set_pixel(int1value, int2value);
      // }
    }

    return 24;
  } else if (memcmp(bufptr, "/cube1/side\0", 12) == 0 && siz == 32) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/side command!");
    #endif

    //
    // 0000   2f 63 75 62 65 30 2f 73 69 64 65 00 2c 69 69 69  /cube0/side.,iii
    // 0010   00 00 00 __ 00 00 00 xx 00 00 00 37 00 00 00 4d                                                  ...........7...M
    //


    if (bufptr[12] == 0x2C && bufptr[13] == 'i' && bufptr[14] == 'i' && bufptr[15] == 'i') {
      #ifdef USE_SERIAL
      // Serial.println("Has correct argument signature");
      #endif

      unsigned side = bufptr[23];
      unsigned b1 = bufptr[27];
      unsigned b2 = bufptr[31];
      // Serial.print("Led ");
      // Serial.print(int1value);
      // Serial.print(" = ");
      // Serial.print(int2value);
      // Serial.print("\n");

      // #ifdef USE_TRELLIS
      // if (side < 6) {
      //  for(int k=0; k<8; k++) {
      //  int bitmask = 1 << k;
      // bits[side*16+0+k] = (b1 & bitmask) == bitmask;
      // bits[side*16+8+k] = (b2 & bitmask) == bitmask;
      display_set_side(side, (b1 << 8) + b2);
      // }
      // displaydirty = true;
      // }
      // #endif
    }   

    return 32;
  } else if (memcmp(bufptr, "/cube1/leds\0", 12) == 0 && siz == 76) {
    #ifdef USE_SERIAL
    // Serial.println("Looks like /cube1/leds command!");
    #endif

    //
    // 0000   2f 63 75 62 65 31 2f 6c 65 64 ?? 00 2c 69 69 69  /cube1/leds.,iii
    // 0010   69 69 69 69 69 69 69 69 69 00 00 00 00 00 00 0a  iiiiiiiii.......
    // 0020   00 00 00 14 00 00 00 1e 00 00 00 28 00 00 00 32  ...........(...2
    // 0030   00 00 00 3c 00 00 00 0a 00 00 00 14 00 00 00 1e  ...<............
    // 0040   00 00 00 28 00 00 00 32 00 00 00 3c              ...(...2...<
    //

    if (bufptr[12] == 0x2C && bufptr[13] == 'i' && bufptr[14] == 'i' && bufptr[15] == 'i' &&
        bufptr[16] == 'i' && bufptr[17] == 'i' && bufptr[18] == 'i' && bufptr[19] == 'i' &&
        bufptr[20] == 'i' && bufptr[21] == 'i' && bufptr[22] == 'i' && bufptr[23] == 'i' &&
        bufptr[24] == 'i') {
      #ifdef USE_SERIAL
      // Serial.println("Has correct argument signature");
      #endif

      uint8_t b[12];

      for(i=0; i<12; i++) {
        b[i] = bufptr[31 + (4 * i)];
      }
      /*
      b[1] = bufptr[31+4];
      b[2] = bufptr[31+8];
      b[3] = bufptr[31+12];

      b[4] = bufptr[31+16];
      b[5] = bufptr[31+20];
      b[6] = bufptr[31+24];
      b[7] = bufptr[63];

      b[8] = bufptr[63+4];
      b[9] = bufptr[63+8];
      b[10] = bufptr[63+12];
      b[11] = bufptr[64+16];
      */

      display_set_all((uint8_t *)&b);
    }

    return 76;
  }
  return 0;
}





static void ICACHE_FLASH_ATTR network_received(void *arg, char *data, unsigned short len) {
  os_printf("udp packet #%d, %d bytes.\n", totalpackets, len);
  parse_osc(data, len);
  totalbytes += len;
  totalpackets ++;
}

struct espconn *pUdpServer;

void udp_listen() {
    pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
    pUdpServer->type = ESPCONN_UDP;
    // pUdpServer->state = ESPCONN_NONE;
    pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    pUdpServer->proto.udp->local_port = 3333;                          // Set local port to 2222
    //pUdpServer->proto.udp->remote_port=3338;                         // Set remote port
    espconn_regist_recvcb(pUdpServer, network_received);
    if(espconn_create(pUdpServer) == 0)
    {
        os_printf("UDP OK\n\r");
    }
}

void wifi_callback( System_Event_t *evt )
{
    os_printf( "%s: %d\n", __FUNCTION__, evt->event );
    
    switch ( evt->event )
    {
        case EVENT_STAMODE_CONNECTED:
        {
            os_printf("connect to ssid %s, channel %d\n",
                        evt->event_info.connected.ssid,
                        evt->event_info.connected.channel);
            break;
        }

        case EVENT_STAMODE_DISCONNECTED:
        {
            os_printf("disconnect from ssid %s, reason %d\n",
                        evt->event_info.disconnected.ssid,
                        evt->event_info.disconnected.reason);
            
         //   deep_sleep_set_option( 0 );
         //   system_deep_sleep( 60 * 1000 * 1000 );  // 60 seconds
            break;
        }

        case EVENT_STAMODE_GOT_IP:
        {
            os_printf("Got IP! " IPSTR ", netmask: " IPSTR ", gateway:" IPSTR,
                        IP2STR(&evt->event_info.got_ip.ip),
                        IP2STR(&evt->event_info.got_ip.mask),
                        IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\n");

//  udp_bind(ptel_pcb, IP_ADDR_ANY, localPort);
 // udp_recv(ptel_pcb, udp_callback, NULL);

          //  espconn_gethostbyname( &dweet_conn, dweet_host, &dweet_ip, dns_done );
            break;
        }
        
        default:
        {
            break;
        }
    }
}





static inline void tcp_random_port_init (void)
{
  extern uint16_t _tcp_new_port_port; // provided by the linker script
  _tcp_new_port_port += xthal_get_ccount () % 4096;
}





bool connectWifi2() {
  bool state = true;
  int i = 0;
  // Serial.enableRX(false) 
//  WiFi.begin(ssid, password);

  ptel_pcb = udp_new();
  
  os_printf("");
  os_printf("Connecting to WiFi");
  os_printf("Connecting...");

  struct station_config conf;
  conf.bssid_set = 0;
  strcpy((char *)conf.ssid, ssid);
  strcpy((char *)conf.password, password);
//  ETS_UART_INTR_DISABLE();
  wifi_set_event_handler_cb( wifi_callback );
  wifi_set_opmode(STATION_MODE);
//  wifi_station_set_auto_connect(0);
  wifi_station_set_reconnect_policy(true);
  wifi_station_set_config(&conf);
//  wifi_station_connect();
//  wifi_station_dhcpc_start();
//  ETS_UART_INTR_ENABLE();

  while ((wifi_get_opmode() & 1) == 0) {
    os_delay_us(500 * 1000);
    os_printf(".");

    if (i > 100) {
      state = false;
      break;
    }

    i++;
  }

  if (state) {
    os_printf("");
    os_printf("Connected to ");
    os_printf(ssid);
 //   os_printf("IP address: ");
//    os_printf(WiFi.localIP());
  } else {
    os_printf("");
    os_printf("Connection failed.");
  }

  return state;
}








void comm_init() {
	// tcp_random_port_init ();
    connectWifi2();
    udp_listen();
}

void comm_update() {

}

void comm_debuginfo() {
    os_printf("Comm total bytes: %d.\n", totalbytes);
}

static void ICACHE_FLASH_ATTR udpNetworkConnectedCb(void *arg) 
{
    struct espconn *conn = (struct espconn *)arg;
//    espconn_regist_recvcb(conn, tcpNetworkRecvCb);

    os_printf("UDP connected\n\r"); 
  //  espconn_sent(conn, last_message, last_message_len);
    // UDP_conn_ok = TRUE;
}

static void ICACHE_FLASH_ATTR udpNetworkReconCb(void *arg, sint8 err) 
{
    os_printf("UDP reconnect\n\r");
    // UDP_conn_ok = FALSE;
    // network_init();
}

static void ICACHE_FLASH_ATTR udpNetworkDisconCb(void *arg) 
{
    os_printf("UDP disconnect\n\r");
    // UDP_conn_ok = FALSE;
    // network_init();
}

static void ICACHE_FLASH_ATTR udpNetworkSentCb(void *arg) 
{
    os_printf("UDP sent\n\r");
    // tcp_conn_ok = FALSE;
    // network_init();
}

void comm_send_osc(uint8_t *bytes, uint8_t len) {
  // int localPort2 = 3333;
  // UDP.beginPacket(remoteIP, localPort2);
  // UDP.write(bytes, len);
  // UDP.endPacket();

    last_message = bytes;
    last_message_len = len;

/*
    last_conn = (struct espconn*)os_zalloc(sizeof(struct espconn));
    last_conn->type=ESPCONN_UDP;                                    // We want to make a TCP connection
    last_conn->state=ESPCONN_NONE;                                  // Set default state to none
    last_conn->proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
    last_conn->proto.udp->local_port=espconn_port();                // Ask a free local port to the API
    last_conn->proto.udp->remote_port=3333;                         // Set remote port (bcbcostam)
    last_conn->proto.udp->remote_ip[0]=192;                          // Your computer IP
    last_conn->proto.udp->remote_ip[1]=168;                           // Your computer IP
    last_conn->proto.udp->remote_ip[2]=1;                         // Your computer IP
    last_conn->proto.udp->remote_ip[3]=107;                         // Your computer IP
*/
    struct espconn tmpconn;
    esp_udp tmpudp;
    memset(&tmpconn, 0, sizeof(struct espconn));
    memset(&tmpudp, 0, sizeof(esp_udp));

    tmpconn.type=ESPCONN_UDP;                                    // We want to make a TCP connection
    tmpconn.state=ESPCONN_NONE;                                  // Set default state to none
    tmpconn.proto.udp = &tmpudp;
    tmpconn.proto.udp->local_port=espconn_port();                // Ask a free local port to the API
    tmpconn.proto.udp->remote_port=3333;                         // Set remote port (bcbcostam)
    tmpconn.proto.udp->remote_ip[0]=192;                          // Your computer IP
    tmpconn.proto.udp->remote_ip[1]=168;                           // Your computer IP
    tmpconn.proto.udp->remote_ip[2]=1;                         // Your computer IP
    tmpconn.proto.udp->remote_ip[3]=107;                         // Your computer IP

    // espconn_regist_connectcb(tmpconn, udpNetworkConnectedCb);   // Register connect callback
    // espconn_regist_disconcb(tmpconn, udpNetworkDisconCb);       // Register disconnect callback
    // espconn_regist_reconcb(tmpconn, udpNetworkReconCb);         // Register reconnection function
    // espconn_regist_sentcb(&tmpconn, udpNetworkSentCb);         // Register reconnection function

    espconn_create(&tmpconn);
    espconn_sent(&tmpconn, bytes, len);
    espconn_disconnect(&tmpconn);

}

void comm_send_key(uint8_t btn, uint8_t down) {
  uint8_t message[24];

  os_printf("send OSC key event; btn=%d, down=%d\n", btn, down);

  //
  // 0000   2f 63 75 62 65 31 2f 6c 65 64 73 00 2c 69 69 00  /cube1/leds.,ii.
  // 0010   00 00 00 06 00 00 00 01                          ........
  //

  message[0] = '/';
  message[1] = 'c';
  message[2] = 'u';
  message[3] = 'b';
  message[4] = 'e';
  message[5] = '1';
  message[6] = '/';
  message[7] = 'b';
  message[8] = 't';
  message[9] = 'n';
  message[10] = 0;
  message[11] = 0;

  message[12] = 0x2C;
  message[13] = 'i';
  message[14] = 'i';
  message[15] = 0;

  message[16] = 0;
  message[17] = 0;
  message[18] = 0;
  message[19] = btn;
  message[20] = 0;
  message[21] = 0;
  message[22] = 0;
  message[23] = down;

  comm_send_osc(message, sizeof(message));
}


