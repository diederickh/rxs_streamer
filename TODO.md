
 rxs_packetizer
 --------------
 - The buffer member is way to big then necessary at this moment. 

 rxs_depacketizer
 ----------------
 - The depacketizer needs to implement a jitter buffer and      
   add the code to correctly construct a receive frame from 
   multiple packets (and ofc. checking for lost packets).