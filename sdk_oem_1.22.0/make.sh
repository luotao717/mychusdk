#!/bin/bash
if [ $# -lt 2 ]
then 
	echo "run like:./make.sh BLINK O1"
	echo "run like:./make.sh GAOKE O1"
	echo "run like:./make.sh WIAIR I1"
	exit -1
fi

file=./package/ioos/src/version
out=./package/ioos/src/firmware
date=`date '+%F %H:%M'`
wanpid=4
reset=1
vendor=$1
product=$2
lanip=192.168.1.1
domain=wiair.cc
ssid=CYWiFi-
firmware=/dev/mtd4
www_dir=www_wiair
export VENDOR=$vendor
export PRODUCT=$product

case "$vendor" in 
	"BLINK")
	wanpid=4
	cp -rf uboot_7620_32m_sdr_vlan.bin uboot.bin
	lanip=192.168.16.1
	domain=blinklogin.cn
	ssid=B-LINK_
	reset=2
	;;
	"GAOKE")
	wanpid=1
	;;
	"WIAIR")
	reset=1
	vendor=CY_WiFi
	firmware=/dev/mtd5
	;;
esac

sed $file -e "
	s/^VENDOR=.*/VENDOR=$vendor/g;
	s/^PRODUCT=.*/PRODUCT=$product/g;
	s/^DATE=.*/DATE=$date/g;
	s/^WANPID=.*/WANPID=$wanpid/g;
	s/^RESET=.*/RESET=$reset/g;
	s/^LANIP=.*/LANIP=$lanip/g;
	s/^DOMAIN=.*/DOMAIN=$domain/g;
	s/^SSID=.*/SSID=$ssid/g;
	s#^FIRMWARE=.*#FIRMWARE=$firmware#g;
	">$out

rm -rf package/base-files/files/www
cp -rf package/web/$www_dir package/base-files/files/www
make V=99
#svn revert $mu

