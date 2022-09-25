#include <stdio.h>
#include <string.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_err.h"

#define STA_SSID "WRITE YOUR SSID"
#define STA_PASSWORD "WRITE YOUR PASSWORD"

#define PORT 3333




static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                int32_t event_id, void* event_data )
{

    printf("RainyDays\n");

}



static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
   
    struct sockaddr_storage dest_addr;
    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }


    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
       	  printf("Unable to create socket: errno\n");
        return;
    }
    int opt = 1;
 

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
	  printf("Socket unable to bind: errno\n");  
        goto CLEAN_UP;
    }
    
    err = listen(listen_sock, 1);
    if (err != 0) {
	    printf("Error occured during listen: errno");
        goto CLEAN_UP;
    }

    while (1) {

     

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
		printf("Unable to accept connection: errno\n");
        
            break;
        }

      

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void app_main(void)
{
	 nvs_flash_init();
	 esp_netif_init();
	 esp_event_loop_create_default();
	 esp_netif_t * ap =  esp_netif_create_default_wifi_sta();
	 esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL);

	 wifi_config_t sta_config = {
	.sta = {
	.ssid =  STA_SSID,
	.password = STA_PASSWORD,
	},
	};


	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	if(ESP_OK == esp_wifi_init(&cfg))
	{
		
		printf("Successfully initialized\n");		
		if(ESP_OK == esp_wifi_set_mode(WIFI_MODE_STA))
		{
			printf("Successfully WIFI SET\n");

			if(ESP_OK == esp_wifi_set_config(WIFI_IF_STA,&sta_config))
                        {

                          printf("Succesfully CONFIGURED\n");
			   if(ESP_OK == esp_wifi_start())
			   {
				   
				printf("Successfully Wifi started\n"); 				
       				printf("Succesfully connected 14:30\n");
       				                                   
				esp_err_t x =   esp_wifi_connect();
				if(ESP_OK == x)
				{
					printf("Succesfully connected\n");
					
					esp_netif_ip_info_t ip_info;					
    					esp_netif_get_ip_info(ap ,&ip_info );	
					printf("My IP: " IPSTR "\n", IP2STR(&ip_info.ip));
    					printf("My GW: " IPSTR "\n", IP2STR(&ip_info.gw));
    					printf("My NETMASK: " IPSTR "\n", IP2STR(&ip_info.netmask));
					xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);			
					
				}
				else if(x == ESP_ERR_WIFI_NOT_INIT)
				{
					printf("ERROR WIFI NOT INIT\n");
				}
				else if( x == ESP_ERR_WIFI_NOT_STARTED)
				{
					 printf("ERROR WIFI NOT STARTED\n");
			        }
				else if( x == ESP_ERR_WIFI_CONN)
				{
					printf("ERROR WIFI CONN\n");
				}
			   
				else if ( x == ESP_ERR_WIFI_SSID)
				{

					printf("ERRORR WIFI SSID\n");
				}

				else{

					printf("NOOO CONNECTED\n");
				   } 

                        }		       
		
	}
	    else{
		  printf("ERROR WIFI SET\n");

	        }
		



	}

}

}




