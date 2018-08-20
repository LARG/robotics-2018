#!/bin/bash

#rc-config delete connman boot
#rc-config delete naopathe default
#rc-config delete lighttpd default

mv /home/nao/scripts/bootprogress_100 /etc/init.d/bootprogress_100
mv /home/nao/scripts/autoload.ini /etc/naoqi/autoload.ini
mv /home/nao/scripts/naoqi /etc/init.d/naoqi

rc-config add utwired boot
rc-config add utwireless boot
rc-config add bootprogress_100 boot
rc-config add naoqi boot

reboot
