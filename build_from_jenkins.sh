#!/bin/bash

/opt/chroots/debian_jessie_x32/mount_jenkins.sh

sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "mkdir -p /root/jenkins/workspace/Console_x32/build"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && cmake .."
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && make"

echo "Build Arduino Host"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "mkdir -p /root/jenkins/workspace/Console_x32/libpiga/hosts/arduino/build"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/libpiga/hosts/arduino/build && cmake .."
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/libpiga/hosts/arduino/build && make"

/opt/chroots/debian_jessie_x32/umount_jenkins.sh
