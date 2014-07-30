/*******************************************************/
/*Optimization notes                    						   */
/*******************************************************/
/*(1.): Use padding buffer as data buffer						   */
/*(2.): Precompute matrices & store them in an array   */
/*(3.): Malloc 820 to "coding" buf & encode(coding+20);*/
/*******************************************************/
/*To Do:*/
/*Verify freeing everywhere*/
/*Check on memory leaks    */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <rxs_streamer/rxs_fec.h>
#include <jerasure.h>
#include <reed_sol.h>

/*The Initializer*/
int rxs_fec_init(rxs_fec* fec, int mode) {

	if (!fec) { return -1; }

	int i;

	/*Initializing buffer to hold 128 packets*/
	fec->buffer = (unsigned char **) malloc(sizeof(unsigned char *) * 128);
	fec->coding = (char **) malloc(sizeof(char *) * fec->numCoding);

	for(i=0; i<128; i++){
		fec->buffer[i] = (unsigned char *) malloc(800);
		bzero(fec->buffer[i], 800);
	}

	/*How many packets do you want returned?*/
	fec->numCoding = 2;

	for(i=0; i<fec->numCoding; i++){
		fec->coding[i] = (char *) malloc(832);
	}

	fec->mode = mode;
	fec->index = 0;

	srand(time(NULL));

	fec->ssrc = rand();
	fec->seqnum = rand();


	return 0;
}

/*Add a packet to the FEC buffer for encoding*/
/*Might convert this function to use gf_complete for encoding manually*/
int rxs_fec_add_packet(rxs_fec* fec, uint8_t* buffer, uint32_t nbytes){

	if(!fec){return -1;}
	if(!buffer){return -2;}
	if(!nbytes){return -3;}

	unsigned char * tmp = (unsigned char *)buffer;
	int i, j, index;
	uint32_t remaining = 800-nbytes;

	printf("The Index!: %d\n", fec->index);

	if(fec->index == 127){

		printf("FEC buffer too full! gotta shut'er down...\n");
		return -4;
	}

	tmp[0]  = 0;
	tmp[0]  = 2 << 6;            /* RTP: version */
	tmp[0] |= 1 << 5;            /* RTP: has padding? */

	memcpy((unsigned char *)fec->buffer[fec->index], (void *)tmp, nbytes);
	bzero(fec->buffer[fec->index]+nbytes, 800-nbytes);

	printf("800-nbytes: %d\n\n", 800-nbytes);

	/*Time to add the padding info at the end...*/

	index = 799;

	while(remaining !=0){

		if(remaining > 255){
			printf("fec->buffer[fec->index][index]: %d\n", fec->buffer[fec->index][index]);
			fec->buffer[fec->index][index] = 255;
			printf("Now: %d\n", fec->buffer[fec->index][index]);
			remaining -= 255;
		}else{
			fec->buffer[fec->index][index] = remaining;
			remaining -= remaining;
		}

		index--;

	}

	/*The below for in a for prints the first 32 bits of the RTP header (4 funz)*/
	//for(j=0; j<4; j++){
	//	for (i = 0; i < 8; i++) {
      //	printf("%d", !!((fec->buffer[fec->index][j] << i) & 0x80));
  //	}
  //	printf("\n");
	//}

	for(j=799; j>795; j--){

		printf("Stored correctly?: %d\n", fec->buffer[fec->index][j]);
	}

	fec->index++;
	return 0;
}

/*******DEFINITELY need to check this*******/
/*Completely free the FEC buffer*/
int rxs_fec_clear_buf(rxs_fec* fec){

	int i;

	for(i=0; i<128; i++){
		free(fec->buffer[i]);
	}

	for(i=0; i<fec->numCoding; i++){
		free(fec->coding[i]);
	}

	free(fec->coding);
	free(fec->buffer);

	return 0;
}

/*******DEFINITELY need to check this*******/
/*Must be called to reset Jerasure encoding/coding buffers */
/*(Number of coding packets sent can be determined dynamically)*/
int rxs_fec_reset_bufs(rxs_fec* fec){

	if(!fec){
		return -1;
	}

//	int i;

//	for(i=0; i<fec->index; i++){

	//	free(fec->data[i]);
//	}

//	for(i=0; i<fec->numCoding; i++){

//		free(fec->coding[i]);
//	}

//	for(i=0; i<fec->numCoding; i++){
//		free(fec->FECbuf[i]);
//	}

//	free(fec->data);
//	free(fec->FECbuf);
//	free(fec->coding);

	fec->index = 0;

	return 0;

}

/*encode the packets in buffer*/
int rxs_fec_encode(rxs_fec* fec){

/*Don't forget to add mode checking...*/

	int *matrix;
	int i;

//	fec->data = (char **) malloc(sizeof(char *) * fec->index);

//	for(i=0; i<fec->index; i++){
//		fec->data[i] = (char *) malloc(800);
//		bzero(fec->data[i], 800);
//	}

	/*Time to memcpy the data*/

//	for(i=0; i<fec->index; i++){
//		memcpy((char *)fec->data[i], (void *) fec->buffer[i], 800);
//	}

	for(i=0; i<fec->numCoding; i++){
		bzero(fec->coding[i], 832);
		fec->coding[i] +=32;
	}

//	printf("Here! fec->coding[0]: %s; fec->data[0]: %s\n",fec->coding[0], fec->data[0] );

	matrix = reed_sol_vandermonde_coding_matrix(fec->index, fec->numCoding, 8);


	jerasure_matrix_encode(fec->index, fec->numCoding, 8, matrix, (char **)fec->buffer, fec->coding, 800);

	for(i=0; i<fec->numCoding; i++){
		fec->coding[i] -=32;
	}
	//free(matrix);

	fec->pkt_span = fec->index;
	/*Time to call rxs_fec_wrap & rxs_fec_reset_bufs...*/
	printf("Succesful encoding! :D gonna call fec->wrap from rxs_streamer...\n");

	return 0;
}

/*Slaps an RTP/FEC header onto the data*/
int rxs_fec_wrap(rxs_fec *fec){

	int i;
//	fec->FECbuf = (uint8_t **) malloc(sizeof(uint8_t *) * fec->numCoding);

//	for(i=0; i<fec->numCoding; i++){
//		bzero(fec->FECbuf[i], 820);
//	}

	for(i=0; i<fec->numCoding; i++){
		fec->coding[i]+=12;
		/*The RTP header******************/
		/*1st Octet*/
		fec->coding[i][0]  = 0;
		fec->coding[i][0]  = 2 << 6;            /* RTP: version */
		fec->coding[i][0] |= 0 << 5;            /* RTP: has padding? */

		/*2nd Octet*/
		fec->coding[i][1]  = 0x7e;

		/*The Sequence Number*/
		uint8_t* v = (uint8_t*)&fec->seqnum;
		fec->coding[i][2] = v[1];
		fec->coding[i][3] = v[0];

		/*The Timestamp*/
		uint8_t* v1 = (uint8_t*)&fec->timestamp;
		fec->coding[i][4] = v1[3];
		fec->coding[i][5] = v1[2];
		fec->coding[i][6] = v1[1];
		fec->coding[i][7] = v1[0];

		/*The SSRC*/
		uint8_t* v2 = (uint8_t*)&fec->ssrc;
		fec->coding[i][8] = v2[3];
		fec->coding[i][9] = v2[2];
		fec->coding[i][10] = v2[1];
		fec->coding[i][11] = v2[0];

		/*The FEC header as specified by:*/
		/*https://tools.ietf.org/id/draft-galanos-fecframe-rtp-reedsolomon-02.txt*/
		fec->coding[i][12] = fec->numCoding;
		fec->coding[i][13] = i;

		/*SN_Base*/
		uint8_t* v3 = (uint8_t*)&fec->SN_Base;
		fec->coding[i][14] = v3[1];
		fec->coding[i][15] = v3[0];

		/*Reserved/ Bit mask length (which is 0)*/
		fec->coding[i][16] = 0;
		fec->coding[i][17] = 0;

		/*pkt_span*/
		uint8_t* v4 = (uint8_t*)&fec->pkt_span;
		fec->coding[i][18] = v4[1];
		fec->coding[i][19] = v4[0];

		/*pkt has been wrapped*/
//		memcpy((uint8_t *)fec->FECbuf[i]+20, (void *)fec->coding[i], 800);
		fec->coding[i]-=12;
		fec->seqnum++;
	}

	return 0;
}

int rxs_fec_unwrap(rxs_fec* fec, uint8_t* buffer, uint32_t nbytes){

	int i;

	return 0;
}
