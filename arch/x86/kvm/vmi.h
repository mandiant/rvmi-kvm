/*
 * KVM VMI support
 *
 * Copyright (C) 2017 FireEye, Inc. All Rights Reserved.
 *
 * Authors:
 *  Jonas Pfoh      <jonas.pfoh@fireeye.com>
 *  Sebastian Vogl  <sebastian.vogl@fireeye.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#ifndef __KVM_X86_VMI_H
#define __KVM_X86_VMI_H

#include <linux/kernel.h>
#include <linux/kvm_host.h>
#include <linux/kvm_vmi.h>
#include <linux/types.h>

#include <asm/vmx.h>

#define KVM_VMI_DEBUG_CRIT           0
#define KVM_VMI_DEBUG_ERROR          1
#define KVM_VMI_DEBUG_WARN           2
#define KVM_VMI_DEBUG_INFO           3
#define KVM_VMI_DEBUG_DEBUG          4

#define KVM_VMI_DEBUG_LEVEL          4

#define kvm_vmi_debug_print(level, prefix, printk_prefix, fmt, ...)         \
       do { if (KVM_VMI_DEBUG_LEVEL >= level)                                 \
                printk(printk_prefix "[ KVM VMI %s ][%s:%s():%d]" fmt,        \
                       prefix, __FILE__, __func__, __LINE__,__VA_ARGS__);          \
       } while(0)

#define kvm_vmi_debug(fmt, ...) \
       kvm_vmi_debug_print(KVM_VMI_DEBUG_DEBUG, "DEBUG", KERN_DEBUG, fmt,     \
                           __VA_ARGS__);
#define kvm_vmi_info(fmt, ...) \
       kvm_vmi_debug_print(KVM_VMI_DEBUG_INFO, "INFO", KERN_INFO, fmt,        \
                           __VA_ARGS__);
#define kvm_vmi_warning(fmt, ...) \
       kvm_vmi_debug_print(KVM_VMI_DEBUG_WARN, "WARNING", KERN_WARNING, fmt,  \
                           __VA_ARGS__);

#define kvm_vmi_error(fmt, ...) \
	kvm_vmi_debug_print(KVM_VMI_DEBUG_ERROR, "ERROR", KERN_ERR, fmt, \
	                    __VA_ARGS__);


#define kvm_vmi_critical(fmt, ...) \
       kvm_vmi_debug_print(KVM_VMI_DEBUG_CRIT, "CRITICAL", KERN_CRIT, fmt,    \
                           __VA_ARGS__);


// Functions in vmx.c
void vmx_vmi_update_execution_controls(u32 exec_control);
u32 vmx_vmi_get_execution_controls(void);
void vmx_vmi_enable_lbr(struct kvm_vcpu *vcpu);
void vmx_vmi_disable_lbr(struct kvm_vcpu *vcpu);

// Functions in vmi.c
void kvm_vmi_vcpu_init(struct kvm_vcpu *vcpu);
void kvm_vmi_vcpu_uninit(struct kvm_vcpu *vcpu);

bool kvm_vmx_task_switch_need_stop(struct kvm_vcpu *vcpu, u64 cr3_out, u64 cr3_in);

int vmx_vmi_feature_control(struct kvm_vcpu *vcpu, union kvm_vmi_feature *feature);

#endif
