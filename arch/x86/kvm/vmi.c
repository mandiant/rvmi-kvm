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

#include <linux/kvm_vmi.h>

#include "vmi.h"

struct vmi_dtb_entry{
	struct hlist_node list;
	u64 dtb;
	bool in;
	bool out;
};

void kvm_vmi_vcpu_init(struct kvm_vcpu *vcpu)
{
	hash_init(vcpu->vmi_dtb_ht);
}

void kvm_vmi_vcpu_uninit(struct kvm_vcpu *vcpu)
{
	int i;
	struct hlist_node *tmp;
	struct vmi_dtb_entry *dtb_entry;

	hash_for_each_safe (vcpu->vmi_dtb_ht, i, tmp, dtb_entry, list) {
		hash_del(&dtb_entry->list);
		kfree(dtb_entry);
	}
}

static struct vmi_dtb_entry* kvm_vmi_dtb_get_entry(struct kvm_vcpu *vcpu, u64 dtb)
{
	struct vmi_dtb_entry *entry;

	hash_for_each_possible(vcpu->vmi_dtb_ht,entry,list,dtb){
		if(entry->dtb == dtb)
			return entry;
	}
	return NULL;
}

static void kvm_vmi_dtb_rm_entry(struct kvm_vcpu *vcpu, u64 dtb)
{
	struct hlist_node *tmp;
	struct vmi_dtb_entry *entry;

	hash_for_each_possible_safe(vcpu->vmi_dtb_ht,entry,tmp,list,dtb){
		if(entry->dtb == dtb){
			hash_del(&entry->list);
			kfree(entry);
			return;
		}
	}
	return;
}

static void kvm_vmi_dtb_add_update_entry(struct kvm_vcpu *vcpu, u64 dtb, bool in, bool out)
{
	struct vmi_dtb_entry *entry;
	struct vmi_dtb_entry *_entry;

	entry = _entry = kvm_vmi_dtb_get_entry(vcpu,dtb);
	if(!entry){
		entry = kzalloc(sizeof(struct vmi_dtb_entry),GFP_KERNEL);
		if(entry == NULL){
			kvm_vmi_error("no memory %p \n",entry);
			return;
		}
	}

	entry->dtb = dtb;
	entry->in = in;
	entry->out = out;

	if(!_entry){
		hash_add(vcpu->vmi_dtb_ht,&entry->list,dtb);
	}
}

bool kvm_vmx_task_switch_need_stop(struct kvm_vcpu *vcpu, u64 cr3_out, u64 cr3_in)
{
	struct vmi_dtb_entry *entry;

	if(cr3_out == cr3_in)
		return false;

	entry = kvm_vmi_dtb_get_entry(vcpu,cr3_out);
	if(entry && entry->out)
		return true;

	entry = kvm_vmi_dtb_get_entry(vcpu,cr3_in);
	if(entry && entry->in)
		return true;

	return false;
}

void vmx_vmi_enable_task_switch_trapping(struct kvm_vcpu *vcpu)
{
	u32 exec_ctls;
	exec_ctls = vmx_vmi_get_execution_controls();
	exec_ctls |= CPU_BASED_CR3_LOAD_EXITING;
	vmx_vmi_update_execution_controls(exec_ctls);
}

void vmx_vmi_disable_task_switch_trapping(struct kvm_vcpu *vcpu)
{
	u32 exec_ctls;
	exec_ctls = vmx_vmi_get_execution_controls();
	exec_ctls &= ~CPU_BASED_CR3_LOAD_EXITING;
	vmx_vmi_update_execution_controls(exec_ctls);
}

int vmx_vmi_feature_control_task_switch(struct kvm_vcpu *vcpu, union kvm_vmi_feature *feature)
{
	struct kvm_vmi_feature_task_switch *ts = (struct kvm_vmi_feature_task_switch*) feature;

	if(ts->enable) {
		if(hash_empty(vcpu->vmi_dtb_ht)){
			vmx_vmi_enable_task_switch_trapping(vcpu);
			vcpu->vmi_feature_enabled[KVM_VMI_FEATURE_TRAP_TASK_SWITCH] = 1;
		}

		kvm_vmi_dtb_add_update_entry(vcpu,ts->dtb,ts->in,ts->out);
	}
	else {
		kvm_vmi_dtb_rm_entry(vcpu,ts->dtb);

		if(hash_empty(vcpu->vmi_dtb_ht)){
			vmx_vmi_disable_task_switch_trapping(vcpu);
			vcpu->vmi_feature_enabled[KVM_VMI_FEATURE_TRAP_TASK_SWITCH] = 0;
		}

	}

	return 0;
}

int vmx_vmi_feature_control_lbr(struct kvm_vcpu *vcpu, union kvm_vmi_feature *feature)
{
	struct kvm_vmi_feature_lbr *lbr = &feature->lbr;

	if (lbr->enable) {
		vcpu->vmi_feature_enabled[KVM_VMI_FEATURE_LBR] = 1;
		vcpu->vmi_lbr_select = lbr->lbr_select;
		vmx_vmi_enable_lbr(vcpu);
	}
	else {
		vcpu->vmi_feature_enabled[KVM_VMI_FEATURE_LBR] = 0;
		vmx_vmi_disable_lbr(vcpu);
	}

	return 0;

}

int vmx_vmi_feature_control_mtf(struct kvm_vcpu *vcpu, union kvm_vmi_feature *feature)
{
	struct kvm_vmi_feature_mtf *mtf = (struct kvm_vmi_feature_mtf*)feature;
	u32 exec_ctls;
	exec_ctls = vmx_vmi_get_execution_controls();

	if(mtf->enable){
		exec_ctls |= CPU_BASED_MONITOR_TRAP_FLAG;
	}
	else{
		exec_ctls &= ~CPU_BASED_MONITOR_TRAP_FLAG;
	}

	vmx_vmi_update_execution_controls(exec_ctls);
	return 0;
}

int vmx_vmi_feature_control(struct kvm_vcpu *vcpu, union kvm_vmi_feature *feature)
{
	int rv = 0;

	switch (feature->feature) {
	case KVM_VMI_FEATURE_TRAP_TASK_SWITCH:
		rv = vmx_vmi_feature_control_task_switch(vcpu, feature);
		break;
	case KVM_VMI_FEATURE_LBR:
		rv = vmx_vmi_feature_control_lbr(vcpu, feature);
		break;
	case KVM_VMI_FEATURE_MTF:
		rv = vmx_vmi_feature_control_mtf(vcpu,feature);
		break;
	default:
		kvm_vmi_warning("unknown feature id %d", feature->feature);
		break;
	}

	return rv;
}
