
 Files
 ````
 - rxs_control.c - used to request lost packets + ask for keyframes (client <> server)
 - rxs_decoder.c - libvpx based decoder
 - rxs_depacketizer.c - unwraps RTP VP8 packets
 - rxs_encoder.c - libvpx based encoder (accepts raw I420 YUV)
 - rxs_generator.c - generates YUV420P frames that can be used while testen w/o webcam
 - rxs_ivf.c - muxer for VP8 frames, can be converted to e.g. webm with avconv
 - rxs_jitter.c - delayed playback buffer, can be used with rxs_control.c to request packets and you have some time to re-receive lost packets
 - rxs_packetizer.c - wraps vp8 data into rtp / rtp-vp8 packets
 - rxs_packets.c - packets buffer 
 - rxs_receiver.c - UDP based receiver; does some network i/o
 - rxs_reconstruct.c - reconstructs VP8 frames that are unwrapped using rxs_depacketizer.c (one vp8 frame is sent using multiple RTP packets) 
 - rxs_sender.c - UDP based sender; does some network i/o
 - rxs_stun.c - basic STUN implementation (udp hole punching)
 - rxs_stun_io.c - networkign for rxs_stun

 ````
 

 References:
 - [Experimental Investigation of the Google Congestion Control for Real Time Flows](http://c3lab.poliba.it/images/0/07/Webrtc_cc-Fhcmn2013.pdf)
 - [Performance Analysis of Receive Side Real Time Congestion Control for WebRTC](http://www.netlab.tkk.fi/~jo/papers/2013-12-pv-webrtc-cc.pdf)
 - [Handling Packet Loss in WebRTC](http://static.googleusercontent.com/media/research.google.com/en//pubs/archive/41611.pdf)
 - [Congestion Control using FEC for Conversational Multimedia Communication](http://www.netlab.tkk.fi/~varun/nagy2014mmsys.pdf)
 - [RFC 5109](http://tools.ietf.org/html/rfc5109)
 - [James S. Plank, FEC library in C](http://web.eecs.utk.edu/~plank/plank/papers/CS-08-627.html)
 - [The World Of Peer 2 Peer, great STUN info](http://en.wikibooks.org/wiki/The_World_of_Peer-to-Peer_(P2P)/Building_a_P2P_System)
 - [FEC Frame for WebRTC](http://www.ietf.org/proceedings/86/slides/slides-86-rtcweb-1.pdf)
 - [Handling Packet Loss in WebRTC](http://2013.ieeeicip.org/proc/pdfs/0001860.pdf)
 - [RFC: Forward Error Correction for WebRTC using FEC FRAME](http://tools.ietf.org/html/draft-mandyam-rtcweb-fecframe-00)
