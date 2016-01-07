#!/bin/bash
if [ $# -lt 2 ]
then 
	echo "./make.sh BLINK BLINK"
	echo "./make.sh GAOKE Q339"
	echo "./make.sh GAOKE W307"
	echo "./make.sh GAOKE Q307R"
	echo "./make.sh GAOKE Q338"
	echo "./make.sh GAOKE W305"
	echo "./make.sh WIAIR I1"
	echo "./make.sh PHICOMM PSG1208"
	exit -1
fi

file=./package/ioos/src/version
out=./package/ioos/src/firmware
kdir=./kernel/linux/
date=`date '+%F %H:%M'`
wanpid=4
reset=1
vendor=$1
product=$2
AC=$3
lanip=192.168.2.1
domain=wiair.cc
ssid=CYWiFi-
telnet_port=2323
firmware=/dev/mtd4
www_dir=www_blink
login_pwd=admin
config=config_4+32
kconfig=$kdir/config_4+32
product_file=product_4_32.h
flash_size=4M

export VENDOR=$vendor
export PRODUCT=$product

case "$vendor" in 
	"BLINK")
	if [ "$PRODUCT" = "$VENDOR" ]; then
		PRODUCT=mt7620_4+32
	fi
	wanpid=4
	cp -rf uboot_blink_32m_sdr.bin uboot.bin
	cp -rf factory_blink.bin factory.bin
	lanip=192.168.16.1
	domain=blinklogin.cn
	ssid=B-LINK_
	reset=2
	;;
	"GAOKE")
	wanpid=1
	sed '5a\config6855Esw LWLLL' $mu>$mu.bak
	cat $mu.bak >$mu
	if [ "$PRODUCT" = "$VENDOR" ]; then
		PRODUCT=mt7620_4+32
	fi
	if [ "$PRODUCT" == "Q307R" ];then
		wanpid=0
		sed '5a\config6855Esw WLLLL' $mu>$mu.bak
	else
		wanpid=1
		sed '5a\config6855Esw LWLLL' $mu>$mu.bak
	fi
	cat $mu.bak > $mu
	cp -rf uboot_7620_32m_ddr1_gpio1.bin uboot.bin
	lanip=192.168.8.1
	ssid=GAOKE_
	domain=gaoke.com
	telnet_port=23
	www_dir=www_gaoke
	login_pwd=gaoke
	;;
	"WIAIR")
	cp -rf uboot_wiair.bin uboot.bin
	cp -rf factory_wiair.bin factory.bin
	sed '5a\config6855Esw LLLLW' $mu>$mu.bak
	cat $mu.bak > $mu
	reset=1
	vendor=CY_WiFi
	VENDOR=CY_WiFi
	www_dir=www_wiair
	firmware=/dev/mtd5
	config=config_8+64
	kconfig=$kdir/config_8+64
	product_file=product_def.h
	flash_size=8M
	;;
	"PHICOMM")
	cp -rf uboot_phicomm.bin uboot.bin
	cp -rf factory_phicomm.bin factory.bin
	#sed '5a\config6855Esw LLLLW' $mu>$mu.bak
	cat $mu.bak > $mu
	reset=1
	domain=phicomm.me
	ssid=PHICOMM_
	firmware=/dev/mtd5
	www_dir=www_phicomm_ac
	config=config_7612e
	kconfig=$kdir/config_7612e
	product_file=product_def.h
	flash_size=8M
	;;
	"HAIER")
	if [ "$PRODUCT" = "$VENDOR" ]; then
		PRODUCT=mt7620_4+32
	fi
	wanpid=4
	sed '5a\config6855Esw LLLLW' $mu>$mu.bak
	cat $mu.bak > $mu
	cp -rf uboot_blink_32m_sdr.bin uboot.bin
	cp -rf factory_blink.bin factory.bin
	lanip=192.168.68.1
	domain=hello.wifi.haier
	ssid=Haier-WiFi-
	www_dir=www_haier
	reset=2
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
	s/^TELNET_PORT=.*/TELNET_PORT=$telnet_port/g;
	s/^LOGIN_PWD=.*/LOGIN_PWD=$login_pwd/g;
	">$out

export FLASH_SIZE=$flash_size
PRODUCT_DIR=$kdir/include/linux/netfilter_ipv4/nf_igd

#cp -rf $PRODUCT_DIR/$product_file $PRODUCT_DIR/product.h
cp -rf $config .config
cp -rf $kconfig $kdir/.config
rm -rf package/base-files/files/www
cp -rf package/web/$www_dir package/base-files/files/www
make V=99
#svn revert $mu

