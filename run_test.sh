#!/bin/bash

size=4096

if [ ! -e /dev/mapper/zero1 ];
then
	sudo dmsetup create zero1 --table "0 $size zero"
	ls -al /dev/mapper/*
	echo $'\n'
fi

if [ ! -e /dev/mapper/dmp1 ];
then
	sudo dmsetup create dmp1 --table "0 $size dmp /dev/mapper/zero1"
fi

ls -al /dev/mapper/*
echo

if [[ -e /dev/mapper/zero1 && -e /dev/mapper/dmp1 ]];
then
	sudo dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1 ;
	echo
	sudo dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1 ;
	echo
	sudo cat /sys/module/dmp/stat/volumes
	echo
fi

echo "...script done"
