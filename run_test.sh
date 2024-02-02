size=4096
if [ ! -e /dev/mapper/zero1 ];
then
	sudo dmsetup create zero1 --table "0 $size zero"
fi

ls -al /dev/mapper/* ;
sudo dmsetup create dmp1 --table "0 $size dmp /dev/mapper/zero1" ;
ls -al /dev/mapper/*

