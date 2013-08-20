/*
 ** Author: Elina Lijouvni
 ** License: GPL v3
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libusb.h>

#ifdef NDEBUG
#define debug_printf(...)
#else
#define debug_printf(...) fprintf(stderr, __VA_ARGS__)
#endif

typedef struct ctx_s ctx_t;
struct ctx_s
{
  libusb_context       *libusb_ctx;
  libusb_device_handle *dev_handle;
};

#define LEAP_VID 0xf182
#define LEAP_PID 0x0003

static void
fprintf_data(FILE *fp, const char * title, unsigned char *data, size_t size)
{
  int i;

  debug_printf("%s", title);
  for (i = 0; i < size; i++) {
    if ( ! (i % 16) )
      debug_printf("\n");
    debug_printf("%02x ", data[i]);
  }
  debug_printf("\n");
}

static void
leap_init(ctx_t *ctx)
{
  int ret;
  unsigned char data[256];

#include "leap_libusb_init.c.inc"
}

int
main(int argc, char *argv[])
{
  ctx_t ctx_data;

  memset(&ctx_data, 0, sizeof (ctx_data));

  ctx_t *ctx = &ctx_data;

  libusb_init( & ctx->libusb_ctx );

  ctx->dev_handle = libusb_open_device_with_vid_pid(ctx->libusb_ctx, LEAP_VID, LEAP_PID);
  if (ctx->dev_handle == NULL) {
    fprintf(stderr, "ERROR: can't find leap.\n");
    exit(EXIT_FAILURE);
  }

  debug_printf("Found leap\n");

  int ret;

  ret = libusb_reset_device(ctx->dev_handle);
  debug_printf("libusb_reset_device() ret: %i: %s\n", ret, libusb_error_name(ret));

  ret = libusb_kernel_driver_active(ctx->dev_handle, 0);
  if ( ret == 1 ) {
    debug_printf("kernel active for interface 0\n");
    libusb_detach_kernel_driver(ctx->dev_handle, 0);
  }
  else if (ret != 0) {
    printf("error\n");
    exit(EXIT_FAILURE);
  }

  ret = libusb_kernel_driver_active(ctx->dev_handle, 1);
  if ( ret == 1 ) {
    debug_printf("kernel active\n");
    libusb_detach_kernel_driver(ctx->dev_handle, 1);
  }
  else if (ret != 0) {
    printf("error\n");
    exit(EXIT_FAILURE);
  }

  ret = libusb_claim_interface(ctx->dev_handle, 0);
  debug_printf("libusb_claim_interface() ret: %i: %s\n", ret, libusb_error_name(ret));

  ret = libusb_claim_interface(ctx->dev_handle, 1);
  debug_printf("libusb_claim_interface() ret: %i: %s\n", ret, libusb_error_name(ret));

  leap_init(ctx);

  debug_printf( "max %i\n",  libusb_get_max_packet_size(libusb_get_device( ctx->dev_handle ), 0x83));

  for ( ; ; ) {
    unsigned char data[16384];
    int transferred;
    ret = libusb_bulk_transfer(ctx->dev_handle, 0x83, data, sizeof(data), &transferred, 1000);
    if (ret != 0) {
      printf("libusb_bulk_transfer(): %i: %s\n", ret, libusb_error_name(ret));
      exit(EXIT_FAILURE);
    }

    debug_printf("read usb frame of %i bytes\n", transferred);

    fwrite(&transferred, sizeof (transferred), 1, stdout);
    fwrite(data, transferred, 1, stdout);
  }

  libusb_exit(ctx->libusb_ctx);

  return (0);
}
