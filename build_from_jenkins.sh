#!/bin/bash

/opt/chroots/debian_jessie_x32/mount_jenkins.sh

rm /var/lib/jenkins/workspace/Console_x32/build/piga-Console-*

sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "mkdir -p /root/jenkins/workspace/Console_x32/build"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && cmake .."
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && make"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && make package"

echo "Add the package to the repository"
cd /media/www/maximaximal.com/www/repos/apt/debian && sudo reprepro includedeb jessie /var/lib/jenkins/workspace/Console_x32/build/piga-Console-*

/opt/chroots/debian_jessie_x32/umount_jenkins.sh
