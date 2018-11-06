#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static int sum;
static int numReplies;
static int avg;


static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
	
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast message received from %d.%d: '%s'\n",
	 from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

	sum += packetbuf_dataptr();
	

	if(numReplies < 9){
	    numReplies++;
	    avg = sum/numReplies;
	    if(numReplies % 2 == 0){
 		printf("Current AVG Temp = %s \n", (char *)avg);
		}
	   
        }
	else{
	printf("Purging replies counter and sum\n");

	numReplies = 0;
	sum = 0;
	}
	
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);
  unicast_open(&uc, 146, &unicast_callbacks);

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("I am your sink", 15);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
    
	
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

