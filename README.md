This module is written for `Linux kernel 6.6.9-200.fc39.x86_64`. Tested and compiled on `Fedora CoreOS` and `GCC 13.2.1`.
# Compilation
```
make
```
# Install
```
sudo insmod dmp.ko
```
Check that the module is installed
```
lsmod | grep dmp
```
# Test
```
./run_test.sh
```
This is what the script does:
```
dmsetup create zero1 --table "0 4096 zero"  //create zero block device
dmsetup create dmp1 --table "0 4096 dmp /dev/mapper/zero1"  //create device mapper proxy
dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1 //write in dmp1 -> zero1
dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1   //read from the dmp1 -> zero1
cat /sys/module/dmp/stat/volumes    //log output
```
You can use any of these commands individually.

For clear your system after testing kernel module. Run
```
./clr.sh
```
The script removes block devices, and it will also ask if you want to remove `the dmp.ko` module
If you choice `yes` and filesystem write smth or kernel a litle bit crush, don't panic it's normal(i think).
This may happen because a module is removed while the kernel is running, it seems it doesn’t really like this.
You can also just reboot, everything will clear itself.