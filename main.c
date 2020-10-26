/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdbool.h>

/* Set period size to 32 frames. */
#define PERIOD_SIZE 32

/* 44100 bits/second sampling rate (CD quality) */
#define SAMPLE_RATE 44100

// Samples are 2 bytes long
#define SAMPLE_MIN -32767
#define SAMPLE_MAX 32767

void loop(int loops, int size, snd_pcm_t *handle, snd_pcm_uframes_t frames);

void play(snd_pcm_t *handle, char *buffer, snd_pcm_uframes_t frames);

int main() {
  long loops;
  int size;
  int rc;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* One channel (mono) */
  snd_pcm_hw_params_set_channels(handle, params, 1);

  val = SAMPLE_RATE;

  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  //frames = PERIOD_SIZE;
  frames = 32;
  // comment
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  size = frames * 2; /* 2 bytes/sample, 1 channel */

  /* We want to loop for 5 seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
  loops = 5000000 / val;

  loop(loops, size, handle, frames);

  snd_pcm_drain(handle);
  snd_pcm_close(handle);

  return 0;
}

void loop(int loops, int size, snd_pcm_t *handle, snd_pcm_uframes_t frames) {
  int rc;
  char sample;
  printf("size: %i\n", size);
  char *buffer = (char *) malloc(size);
  bool min_max = true;
  //while (loops > 0) {
  while (1) {
    //loops--;
    sample = min_max ? 0x80 : 0x00;
    for (int i = 0; i < size; i++) {
      *(buffer + i) = sample;
    }
    min_max = !min_max;

    //rc = read(0, buffer, size);
    //if (rc == 0) {
    //  fprintf(stderr, "end of file on input\n");
    //  break;
    //} else if (rc != size) {
    //  fprintf(stderr,
    //          "short read: read %d bytes\n", rc);
    //}
    play(handle, buffer, frames);
  }
  free(buffer);
}

void play(snd_pcm_t *handle, char *buffer, snd_pcm_uframes_t frames) {
  int rc = snd_pcm_writei(handle, buffer, frames);
  if (rc == -EPIPE) {
    /* EPIPE means underrun */
    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(handle);
  } else if (rc < 0) {
    fprintf(stderr,
            "error from writei: %s\n",
            snd_strerror(rc));
  }  else if (rc != (int)frames) {
    fprintf(stderr,
            "short write, write %d frames\n", rc);
  }
}
