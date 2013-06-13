
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <cv.h>
#include <highgui.h>

typedef struct ctx_s ctx_t;
struct ctx_s
{
  int quit;
};

#if 0
static void
fprintf_data(FILE *fp, const char * title, unsigned char *data, size_t size)
{
  int i;

  printf("%s", title);
  for (i = 0; i < size; i++) {
    if ( ! (i % 16) )
      printf("\n");
    printf("%02x ", data[i]);
  }
  printf("\n");
}
#endif

#define VFRAME_WIDTH  640
#define VFRAME_HEIGHT 240
#define VFRAME_SIZE   (VFRAME_WIDTH * VFRAME_HEIGHT)

typedef struct frame_s frame_t;
struct frame_s
{
  IplImage* frame;
  uint32_t id;
  uint32_t data_len;
  uint32_t state;
};

#define UVC_STREAM_EOF                                  (1 << 1)

static void
process_video_frame(ctx_t *ctx, frame_t *frame)
{
  int key;

  cvShowImage("mainWin", frame->frame );
  key = cvWaitKey(1);
  if (key == 'q' || key == 0x1B)
    ctx->quit = 1;
}

static void
process_usb_frame(ctx_t *ctx, frame_t *frame, unsigned char *data, int size)
{
  int i;

  int bHeaderLen = data[0];
  int bmHeaderInfo = data[1];

  uint32_t dwPresentationTime = *( (uint32_t *) &data[2] );
  //printf("frame time: %u\n", dwPresentationTime);

  if (frame->id == 0)
    frame->id = dwPresentationTime;

  for (i = bHeaderLen; i < size ; i += 2) {
    if (frame->data_len >= VFRAME_SIZE)
      break ;

    CvScalar s;
    s.val[2] = data[i];
    s.val[1] = data[i+1];
    s.val[0] = 0;
    int x = frame->data_len % VFRAME_WIDTH;
    int y = frame->data_len / VFRAME_WIDTH;
    cvSet2D(frame->frame, 2 * y,     x, s);
    cvSet2D(frame->frame, 2 * y + 1, x, s);
    frame->data_len++;
  }

  if (bmHeaderInfo & UVC_STREAM_EOF) {
    //printf("End-of-Frame.  Got %i\n", frame->data_len);

    if (frame->data_len != VFRAME_SIZE) {
      //printf("wrong frame size got %i expected %i\n", frame->data_len, VFRAME_SIZE);
      frame->data_len = 0;
      frame->id = 0;
      return ;
    }

    process_video_frame(ctx, frame);
    frame->data_len = 0;
    frame->id = 0;
  }
  else {
    if (dwPresentationTime != frame->id && frame->id > 0) {
      //printf("mixed frame TS: dropping frame\n");
      frame->id = dwPresentationTime;
      /* frame->data_len = 0; */
      /* frame->id = 0; */
      /* return ; */
    }
  }
}


int
main(int argc, char *argv[])
{
  ctx_t ctx_data;

  memset(&ctx_data, 0, sizeof (ctx_data));

  ctx_t *ctx = &ctx_data;

  cvNamedWindow("mainWin", 0);
  cvResizeWindow("mainWin", VFRAME_WIDTH, VFRAME_HEIGHT * 2);

  frame_t frame;
  memset(&frame, 0, sizeof (frame));
  frame.frame = cvCreateImage( cvSize(VFRAME_WIDTH, 2 * VFRAME_HEIGHT), IPL_DEPTH_8U, 3);

  for ( ; ; ) {
    unsigned char data[16384];
    int usb_frame_size;

    if ( feof(stdin) || ctx->quit )
      break ;

    fread(&usb_frame_size, sizeof (usb_frame_size), 1, stdin);
    fread(data, usb_frame_size, 1, stdin);

    process_usb_frame(ctx, &frame, data, usb_frame_size);
  }

  cvReleaseImage(&frame.frame);

  return (0);
}
