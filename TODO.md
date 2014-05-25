
        
 general
 -------
 - make user of a generic rxs_packet to represent a VP8 packet. 

 rxs_packetizer
 --------------
 - The buffer member is way to big then necessary at this moment. 

 rxs_depacketizer
 ----------------
 - The depacketizer needs to implement a jitter buffer and      
   add the code to correctly construct a receive frame from 
   multiple packets (and ofc. checking for lost packets).

 rxs_control
 -----------
 - Better send buffer management (shouldn't allocate on heap
   for commands, see note at control_sender_send. )

 rxs_jitter
 ----------
 - Implement a rxs_jitter_clear() that deallocates the buffer + packets.