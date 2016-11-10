// minimp3 example player application for Linux/OSS
// this file is public domain -- do with it whatever you want!
#include "minimp3.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

size_t strlen(const char *s);

#define DECODE_FRAME_SIZE 4096
int main(int argc, char *argv[]) {
    mp3_decoder_t mp3;
    mp3_info_t info;
	FILE *fp;
	FILE *fp_wav;
    unsigned char *stream_pos;
	unsigned char *stream_pos_hold;
    signed short sample_buf[MP3_MAX_SAMPLES_PER_FRAME];
    int bytes_left;
	int decode_frame = DECODE_FRAME_SIZE;
	
    int frame_size;
	//int i = 0;

    printf("minimp3 -- a small MPEG-1 Audio Layer III player based on ffmpeg\n\n");
    if (argc < 2) {
        printf("Error: no input file specified!\n");
        return 1;
    }
	
	fp = fopen(argv[1], "rb");
	if(NULL == fp)
    {
        printf("\n fopen() Error!!!\n");
        return -1;
    }
	fp_wav = fopen("Op.pcm", "wb");
	if(NULL == fp)
    {
        printf("\n Op fopen() Error!!!\n");
        return -1;
    }
	fseek(fp, 0L, SEEK_END);
	bytes_left = ftell(fp);
	rewind(fp);  	
	printf("bytes left is %d\n", bytes_left);	
	stream_pos = (unsigned char*)malloc(decode_frame*sizeof(unsigned char));
	stream_pos_hold = stream_pos;
	if(sizeof(unsigned char)*decode_frame != fread(stream_pos,sizeof(unsigned char),decode_frame,fp))
    {
        printf("\n fread() failed\n");
        return 1;
    }
    printf("Now Playing: ");
    printf("%s\n",argv[1]);

    mp3 = mp3_create();
    frame_size = mp3_decode(mp3, stream_pos, decode_frame, sample_buf, &info);
	printf("Frame size is %d\n", frame_size);
    if (!frame_size) {
        printf("\nError: not a valid MP3 audio file!\n");
        return 1;
    }
    
    #define FAIL(msg) { \
        printf("\nError: " msg "\n"); \
        return 1; \
    }  

    printf("\n\nPress Ctrl+C to stop playback.\n");
	
	printf("No of channels :- %d\n", info.channels);
	printf("Audio sample rate :- %d\n", info.sample_rate);
	
	while((bytes_left >= 0) && (frame_size >= 0))
	{
		while ((decode_frame > 0) && (frame_size > 0)) {
			stream_pos += frame_size;
			decode_frame -= frame_size;
			//printf("Audio bytes is:- %d\n",info.audio_bytes );
			fwrite(sample_buf, 1, info.audio_bytes, fp_wav);
			frame_size = mp3_decode(mp3, stream_pos, decode_frame, sample_buf, NULL);
			//printf("Frame int size is %d\n", frame_size);
		}
		bytes_left = bytes_left - DECODE_FRAME_SIZE + decode_frame;
		fseek(fp, -(decode_frame), SEEK_CUR);
		
		stream_pos = stream_pos_hold;
		decode_frame = DECODE_FRAME_SIZE;
		fread(stream_pos,sizeof(unsigned char),decode_frame,fp);
		frame_size = mp3_decode(mp3, stream_pos, decode_frame, sample_buf, NULL);
		
		/*for(i=0;i<info.audio_bytes;i++)
		{
			printf("%x ",sample_buf[i]);
		}*/
	}
	fclose(fp_wav);
	free(stream_pos);
	mp3_done(mp3);
    return 0;
}