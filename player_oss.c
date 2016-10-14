// minimp3 example player application for Linux/OSS
// this file is public domain -- do with it whatever you want!
#include "libc.h"
#include "minimp3.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

size_t strlen(const char *s);
//#define printf(text) write(1, (const void *) text, strlen(text))

int main(int argc, char *argv[]) {
    mp3_decoder_t mp3;
    mp3_info_t info;
    int fd, pcm;
    void *file_data;
    unsigned char *stream_pos;
    signed short sample_buf[MP3_MAX_SAMPLES_PER_FRAME];
    int bytes_left;
    int frame_size;
    int value;

    printf("minimp3 -- a small MPEG-1 Audio Layer III player based on ffmpeg\n\n");
    if (argc < 2) {
        printf("Error: no input file specified!\n");
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("Error: cannot open `");
        printf("%s\n",argv[1]);
        printf("'!\n");
        return 1;
    }
    
    bytes_left = lseek(fd, 0, SEEK_END);   	
	printf("bytes left is %d\n", bytes_left);	
    file_data = mmap(0, bytes_left, PROT_READ, MAP_PRIVATE, fd, 0);
    stream_pos = (unsigned char *) file_data;
    bytes_left -= 100;
    printf("Now Playing: ");
    printf("%s\n",argv[1]);

    mp3 = mp3_create();
    frame_size = mp3_decode(mp3, stream_pos, bytes_left, sample_buf, &info);
	printf("Frame size is %d\n", frame_size);
    if (!frame_size) {
        printf("\nError: not a valid MP3 audio file!\n");
        return 1;
    }
    
    #define FAIL(msg) { \
        printf("\nError: " msg "\n"); \
        return 1; \
    }   

    pcm = open("/dev/dsp", O_WRONLY);
    if (pcm < 0) FAIL("cannot open DSP");

    value = AFMT_S16_LE;
    if (ioctl(pcm, SNDCTL_DSP_SETFMT, &value) < 0)
        FAIL("cannot set audio format");

    if (ioctl(pcm, SNDCTL_DSP_CHANNELS, &info.channels) < 0)
        FAIL("cannot set audio channels");

    if (ioctl(pcm, SNDCTL_DSP_SPEED, &info.sample_rate) < 0)
        FAIL("cannot set audio sample rate");

    printf("\n\nPress Ctrl+C to stop playback.\n");

    while ((bytes_left >= 0) && (frame_size > 0)) {
        stream_pos += frame_size;
        bytes_left -= frame_size;
		printf("bytes left is %d\n", bytes_left);
        write(pcm, (const void *) sample_buf, info.audio_bytes);
        frame_size = mp3_decode(mp3, stream_pos, bytes_left, sample_buf, NULL);
		printf("Frame size is %d\n", frame_size);
		
    }

    close(pcm);
    return 0;
}
