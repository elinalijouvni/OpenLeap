OpenLeap
========

An initiative for an open source driver for the Leap Motion sensor.


I recently got my hands on Leap Motion devices [1].

The so-called "Linux support" is in fact only a x86/x64 Ubuntu 12.04
LTS proprietary binaries. So there is no luck for other
platform/system aficionados.

Since the device may become mainstream, with the Leap motion HP deal
for example [2], an open source alternative to access to the device
would be welcome. Moreover, it would be nice to see the device work on
new kind of devices, like Android/ARM platforms. I initially wanted to
wait until the release of the Leap Motion to publish this code.  The
release date was postponed [3] and I waited too long, so here it is.

Plugging the device will present a standard USB Video Class device
(like a USB webcam). A modern system even create the /dev/videoN
device for you, but trying grab a video stream from this device will
result to errors or black images.

By looking a bit to the Leap SDK, I saw lots of open source
BSD-like packages: libusbk, Qt4, turbojpeg, OpenSSL...

I was a bit disappointed to see an almost standard device using so
many open source BSD code not working on an open system. So I decided
to dig a bit to make the device work. Initially I wanted to access the
raw depth map, a bit like OpenNI for the Kinect. It turned out it's
not directly possible since the leap motion hardware seems to be very
simple (or simpler than the Kinect, at least). From what I understood,
the leap motion device is "only" a stereo infra-red UVC webcam with a
wide-angle lens. For an announced price of 80$ I was expecting
something more complex. I may have missed something, but the real
magic seems to be in the software part. I looked a bit in the
device. The main components are: a Cypress EZ-USB FX3 controller [4],
a serial EEPROM [5] [6], two camera sensors (not sure if it's CCD,
CMOS or something else), few IR LEDs and a bunch of small components
linking everything together. For understanding FX3 based devices,
Cypress Application Notes are a good source of information [7], more
especially the image sensor one [8].

I wrote two small programs to access to the device:

* "low-level-leap": for initializing the leap motion device and
  grabbing raw data on its standard output.

* "display-leap-data-opencv": which read the raw data from its
  standard input, unpack frames, and display them.  There is also
  a sdl version.

These two programs are very simple proof-of-concept code. Since this
was coded in few hours, the code quality is really poor. Some frame
drop / image breaks may appear due to the synchronous I/O.

For the device initialization, I did not reverse engineered the device
communication. I just captured a normal initialization of the Leap SDK
with Wireshark, and made a Wireshark Lua script to generate libusb C
code to replay the data. I expect there is some calibration in there
and a replayed initialization like this may result to a poor
functioning. But it's a start. I don't include my initialization file,
because I don't know what kind of data there is in it (a device serial
number for example).

I saw two operating modes of the device: one with 640x240 frames with
IR LEDs always ON. And one other with 640x120 frames with IR LEDs
going ON and OFF at each frame. I suppose this 2nd mode is the "bad
lightning / robust mode" (for subtracting ambient IR sources).  To be
sure, one has to play with the control panel of the device, and see
what happen on the USB bus. Moreover, the device advertise other
resolutions in its descriptor, but I didn't saw them during my testing
which was USB 2.0 only. Maybe they are used in USB 3.0.

Frames are not compressed. Each pixel has two components of 8 bits
each. The two components are the Left/Right camera pixels (i.e. pixels
from the two cameras are interleaved). Each component is the IR
illumination. The last few pixels are data that does not seems to be
the illumination.

Moreover, the Leap SDK may need to download the device firmwares and
some "calibration" data. I currently have no idea what it is. Could be
lens distortion info, learning data sets, etc...

How to see Leap Motion images:

* Plug the Leap,
* run the command: "lsusb -d f182:0003" and note the bus and device address.
  Check if your device has a different VID/PID. Replace it if needed.
* This step is needed only if there is no udev rule file installed.
  Change device permission with the command, as root:
  chmod 666 /dev/bus/usb/BUS/DEV_ADDRESS
  (replace BUS and DEV_ADDRESS with values from the previous command).
* in case the usbmon kernel module is not loaded, run: "modprobe usbmon" as root
* Run a capture of the USB bus traffic, with the command, as root:
  "tshark -i usbmonX -w leap-init.pcap" where X is the number of the USB bus.
  You may also use the graphical Wireshark as well.
* Run the original Leap software, make sure the device is properly detected and initialized.
  If unsure, test with the visualizer that everything is working.
* Stop the Leap software and the capture.
* Generate the Leap initialization C code with the command:
  "make_leap_usbinit.sh DEV_ADDRESS leap-init.pcap > leap_libusb_init.c.inc"
* Build the low-level-leap and display-leap-data-opencv programs with the "make" command.
* You should be able to see images from the Leap device with the command:
  "./low-level-leap | ./display-leap-data-opencv"

  If there is lots of frame skips/break, try the following things:
* close some CPU consuming applications;
* set the CPU frequency governor to "performance";
* add some poor buffering with: "./low-level-leap | cat | ./display-leap-data-opencv";
* work offline:  "./low-level-leap > leap.dat" then "./display-leap-data-opencv < leap.dat";
* implement it the right way (async IO, ring buffer, threads)

I don't think the publication of this code will do any harm to the
Leap Motion company, since it's only a low level frame display, and
there is no finger tracking code. On the other hand, I would like to
see if the community can come up with other ideas with the device.
Ideally, the Leap Motion could also give few details for a low level
acces to the device, or maybe a simpler driver than the current SDK.
An access to the depth map would be nice too, if possible.

I publish these small code snippets under the GPL v3 license, to make
sure that the community will come with something nicer, hopefully.

Since I will not have lots of time to put on this little project, I
expect some people in the community to continue it. Here is some
hints for future works:

* Make a proper driver based on the uvcvideo driver. Maybe it's
  possible to add some quirks into the kernel driver: it would be nice
  to see the leap motion as a V4L2 video input. See [9] [10].

* Play with OpenCV stereo image calibration to reconstruct a depth map
  from the video flow. See [11] [12].


Have fun!


Elina.


[1]. https://www.leapmotion.com/

[2]. http://www.bbc.co.uk/news/technology-22166424

[3]. http://blog.leapmotion.com/post/48872742284/release-date-update

[4]. Cypress EZ-USB FX3 datasheet:
     http://www.cypress.com/?mpn=CYUSB3014-BZXC

[5]. FLASH Memory: Newer leap (rev5+?) 32Mb version, MX25L32 datasheet:
     http://www.mxic.com.tw/QuickPlace/hq/PageLibrary4825740B00298A3B.nsf/h_Index/6F878CF760C559BD482576E00022E6CC/?OpenDocument&EPN=MX25L3206E

[6]. FLASH Memory: Early version 8Mb version, m25p80 datasheet:
     http://www.micron.com/parts/nor-flash/serial-nor-flash/m25p80-vmw6g?pc={6BAC3112-DFB2-4ADB-BAB0-158F28467649}

[7]. Cypress EZ-USB FX3 application notes:
     http://www.cypress.com/?id=3526&rtID=76

[8]. Cypress EZ-USB FX3 application note: AN75779 - How to Implement an Image Sensor Interface with EZ-USB FX3 in a USB Video Class (UVC) Framework
     http://www.cypress.com/?rID=62824

[9]. USB Video Class 1.1 specification:
     http://www.usb.org/developers/devclass_docs/USB_Video_Class_1_1_090711.zip

[10]. Linux kernel uvcvideo driver:
      https://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/drivers/media/usb/uvc

[11]. OpenCV StereoBM operator:
      http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html#stereobm-operator

[12]. OpenCV stereo image match sample:
      https://github.com/Itseez/opencv/blob/master/samples/cpp/stereo_match.cpp
