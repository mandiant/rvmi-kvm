![rVMI Logo](/resources/rvmi-kvm.png)

# rVMI - KVM

This is a fork of Linux-KVM that includes the **rVMI** extensions.

In the following, we will provide a brief overview of rVMI with a focus
on the Linux-KVM extensions. If you are looking for the main rVMI repository
please go to <https://github.com/fireeye/rvmi/>.

If you are interested in Linux-KVM go to <https://www.linux-kvm.org>.

## About

rVMI is a debugger on steroids. It leverages Virtual Machine Introspection (VMI)
and memory forensics to provide full system analysis. This means that an analyst
can inspect userspace processes, kernel drivers, and preboot environments in a
single tool.

It was specifially designed for interactive dynamic malware analysis. rVMI isolates
itself from the malware by placing its interactive debugging environment out of the
virtual machine (VM) onto the hypervisor-level. Through the use of VMI the analyst
still has full control of the VM, which allows her to pause the VM at any point in
time and to use typical debugging features such as breakpoints and watchpoints. In
addtion, rVMI provides access to the entire Rekall feature set, which enables an
analyst to inspect the kernel and its data structures with ease.

## Installing KVM with rVMI

**WARNING:** rVMI is currently only compatible with Intel CPUs, these instructions only
consider the replacement of modules compatible with Intel CPUs.  Additionally, these
steps will replace the kvm kernel modules on your system.

For now, if you are interested in installing on kernel versions for which we have no
branch, you will have to rebase the VMI changes onto the appropriate upstream branch
yourself.

Begin by checking out the branch appropriate for your kernel (We will be using the
linux-4.4.y-rvmi)

```
$ git clone https://github.com/fireeye/rvmi-kvm.git rvmi-kvm
$ cd rvmi-kvm
$ git checkout linux-4.4.y-rvmi
```

At this point you will have to copy your current kernel config into this folder.
Generally, the config can be found in the /boot/ directory.  The name and location of
this config may vary depending on your Linux distribution.

```
$ cp /boot/config-`uname -r` .config
$ cp /usr/src/linux-headers-$(uname -r)/Module.symvers .
```

Having done this, you will need to configure and prepare the kernel source once.

```
$ yes "" | make oldconfig
$ make prepare
$ make scripts
```

Once prepared, subsequent compilation of the kvm modules can be completed with:

```
$ make modules SUBDIR=arch/x86/kvm/
```

The generated modules will replace your current KVM modules.  In order to replace
them temporarily, follow these steps:

```
$ sudo rmmod kvm-intel
$ sudo rmmod kvm
$ sudo insmod arch/x86/kvm/kvm.ko
$ sudo insmod arch/x86/kvm/kvm-intel.ko
```

If you would like to replace them permanently, please follow these steps (we
recommend you first try replacing the modules temporarily to make sure they work):

```
$ sudo cp arch/x86/kvm/kvm.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm.ko
$ sudo cp arch/x86/kvm/kvm-intel.ko /lib/modules/$(uname -r)/kernel/arch/x86/kvm/kvm-intel.ko
$ sudo modprobe -r kvm-intel
$ sudo modprobe -r kvm
$ sudo modprobe kvm
$ sudo modprobe kvm-intel
```
## Using rVMI

To run rVMI please follow the instructions located at <https://github.com/fireeye/rvmi/>.

## Linux-KVM

KVM (for Kernel-based Virtual Machine) is a full virtualization solution for Linux
on x86 hardware containing virtualization extensions (Intel VT or AMD-V). It
consists of a loadable kernel module, kvm.ko, that provides the core virtualization
infrastructure and a processor specific module, kvm-intel.ko or kvm-amd.ko.

Using KVM, one can run multiple virtual machines running unmodified Linux or Windows
images. Each virtual machine has private virtualized hardware: a network card, disk,
graphics adapter, etc.

KVM is open source software. The kernel component of KVM is included in mainline
Linux, as of 2.6.20.

## Licensing and Copyright

Linux is a trademark of Linus Torvalds  
Copyright 2017 FireEye, Inc. All Rights Reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation. Version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.

## Bugs and Support

There is no support provided. There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

If you think you've found a bug particular to rvmi-kvm, please report it at:

https://github.com/fireeye/rvmi-kvm/issues

In order to help us solve your issues as quickly as possible,
please include the following information when filing a bug:

* The version of rvmi-kvm you're using
* The kernel version you are using
* The guest operating system you are analyzing
* The complete command line you used to run rvmi-qemu
* The exact steps required to reproduce the issue

If you think you have found a bug in one of the other rvmi components, please report appropriately:

https://github.com/fireeye/rvmi-qemu/issues  
https://github.com/fireeye/rvmi-rekall/issues

If you are not sure or would like to file a general bug, please report here:

https://github.com/fireeye/rvmi/issues

## More documentation

Further documentation is available at
https://github.com/fireeye/rvmi/
