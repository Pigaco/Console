#!/bin/bash

/opt/chroots/debian_jessie_x32/mount_jenkins.sh

sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "mkdir -p /root/jenkins/workspace/Console_x32/build"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "rm -f /root/jenkins/workspace/Console_x32/build/piga-pigaco-*"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && cmake .."
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && make"
sudo chroot /opt/chroots/debian_jessie_x32 /bin/bash -c "cd /root/jenkins/workspace/Console_x32/build && make package"

echo "Add the package to the repository"
PIGA_DEB="$(ls /var/lib/jenkins/workspace/Console_x32/build/*deb)"
echo "DEBFILE: $PIGA_DEB"
cd /media/maximaximal.com/www/repos/apt/debian && sudo reprepro includedeb jessie "$PIGA_DEB"

/opt/chroots/debian_jessie_x32/umount_jenkins.sh
