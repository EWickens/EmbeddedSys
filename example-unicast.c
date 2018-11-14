//Eoin Wickens
//R00151204
//eoin.wickens@mycit.ie


#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include "dev/sht11/sht11-sensor.h"

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
//Unicast Callback Function
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast message received from %d.%d\n",
	 from->u8[0], from->u8[1]);
}
/*---------------------------------------------------------------------------*/
//Unicast Sent Details Function
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

  if(linkaddr_cmp(dest, &linkaddr_null)) {
    return;
  }
  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
    dest->u8[0], dest->u8[1], status, num_tx);
}
/*---------------------------------------------------------------------------*/

//Unicast Callback Struct
static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};

//Unicast Connection Struct
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/

static int avg; //Rolling Average
static int sumArray[5];//Array to hold 5 seperate temperature readings
static int sum; //Sum of the five readings

static int tempVal;//Temperature Value


//Upon receving a broadcast, a reply is sent out via unicast
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  //Displays the address the broadcast was sent from
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());


    //Reply via unicast
    linkaddr_t addr;
    char str[5];
    sprintf(str, "%d", avg);
    
    //Sends the reply to the same address the broadcast came from
    packetbuf_copyfrom(str, 5);
    addr.u8[0] = from->u8[0];
    addr.u8[1] = from->u8[1];

    if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
    unicast_send(&uc, &addr);
    }

}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

/*---------------------------------------------------------------------------*/



PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
    
  PROCESS_BEGIN();
  //Cant initialize an integer in the for loop in C99
  int i = 0;
  
  //Populates the array with 0's to stop null pointer exceptions
  for(i = 0; i < 5; i++){
	  sumArray[i] = 0;
  }
  
  //Opens up the channels for callbacks
  unicast_open(&uc, 146, &unicast_callbacks);
  broadcast_open(&broadcast, 129, &broadcast_call);
 

  //Main loop
  while(1) {
   
    static struct etimer et;
    //Activates sensor
    SENSORS_ACTIVATE(sht11_sensor);
    
    etimer_set(&et, CLOCK_SECOND);
    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	
    int val = sht11_sensor.value(SHT11_SENSOR_TEMP);
    //gets the temperature in Celcius
    tempVal = (-39.60 + 0.01*val);


    int j = 0;
    sum = 0;
	    //Moves the positions in the array to compress it to keep rolling average
            for(j = 0; j < 5; j++){
		sum = sum + sumArray[j]; 
	   	 if((j + 1) < 5){
	   	 sumArray[j] = sumArray[j + 1];
    	         }
  	    }
	    //Adds the latest value into the array after it has been compressed
	    sumArray[4] = tempVal;
	    
	    //Calculates the average
	    avg = sum/5;
	    

    etimer_reset(&et);
    SENSORS_DEACTIVATE(sht11_sensor);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

