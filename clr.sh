#!/bin/bash

sudo dmsetup remove dmp1
sudo dmsetup remove zero1

read -p $'Be careful! Removing kernel module can be crush your system. You can just reboot =)\nContinue (y/n)?' choice
case "$choice" in
  y|Y ) echo "yes"
	sudo rmmod dmp
	;;
  n|N ) echo "no"
	;;
  * ) echo "invalid. Script done";;
esac
