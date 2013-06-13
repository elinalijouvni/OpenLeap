#! /bin/bash

if [ $# -ne 2 ] ; then
    echo "usage: $0 <leap_usb_dev_address> <pcap_file>"
    exit
fi

export LEAP_DEV_ADDR=$1
PCAP_FILE=$2

tshark \
  -r ${PCAP_FILE} \
  -T fields \
  -e none \
  -R "usb.device_address == ${LEAP_DEV_ADDR} and usb.transfer_type == 0x02 and usb.endpoint_number.direction == 0 and usb.setup.bRequest == 1" \
  -Xlua_script:usb_c.lua
