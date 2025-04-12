#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/vmx.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include "tinyvmm.h"

#define MSR_IA32_FEATURE_CONTROL 0x3A

static DEFINE_PER_CPU(void *, vmxon_region);


static u32 get_vmcs_revision_id(void)
{
    u32 low, high;
    rdmsr(MSR_IA32_VMX_BASIC, low, high);
    return low & 0x7FFFFFFF;
}

int cpu_has_vmx (void) {
    int eax, ebx, ecx, edx;

    asm volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)     // Output operands.
        : "a"(1)                                        // Input operands.
    );

    if( ecx & ( 1 << 5 )) {                             // check ecx 5th bit. if 1 VMX supported, 0 VMX not supported.
        return 1;
    }

    return 0;
}


void vmx_enable_on_cpu (void *unused) {
    u64 feature_control;
    u64 cr4;
    u8 status;
    void *region;
    phys_addr_t phys;

    // 1. Enable VMX in CR4
    cr4 = __read_cr4();
    __write_cr4(cr4 | X86_CR4_VMXE);

    // 2. Check VMX Lock Bit
    rdmsrl(MSR_IA32_FEATURE_CONTROL, feature_control);
    if (!(feature_control & 0x1)) {
        // Lock bit not set, try setting it
        feature_control |= 0x5;  // Lock + VMXON inside SMX=0
        wrmsrl(MSR_IA32_FEATURE_CONTROL, feature_control);
    }

    // 3. Allocate and prepare VMXON region
    region = (void *)__get_free_page(GFP_KERNEL | GFP_DMA);
    if (!region) {
        PR_ERR("Failed to allocate VMXON region\n");
        return;
    }

    memset(region, 0, PAGE_SIZE);
    *(u32 *)region = get_vmcs_revision_id(); // writing VMCS revision id

    per_cpu(vmxon_region, smp_processor_id()) = region;
    phys = virt_to_phys(region);

    // 5. Execute VMXON
    asm volatile (
        "vmxon %[pa]; setna %0"
        : "=rm"(status)
        : [pa] "m"(phys)
        : "cc", "memory"
    );

    if (status) {
        PR_ERR("VMXON failed on CPU %d\n", smp_processor_id());
        free_page((unsigned long)region);
        per_cpu(vmxon_region, smp_processor_id()) = NULL;
    } else {
        PR_INFO("VMXON successful on CPU %d\n", smp_processor_id());
    }
}

void vmx_disable_on_cpu(void *unused)
{
    void *region;
    u64 cr4;
    // Execute VMXOFF
    asm volatile ("vmxoff" ::: "cc", "memory");

    // Free the VMXON region
    region = per_cpu(vmxon_region, smp_processor_id());
    if (region) {
        free_page((unsigned long)region);
        per_cpu(vmxon_region, smp_processor_id()) = NULL;
    }

    // Clear VMXE in CR4
    cr4 = __read_cr4();
    __write_cr4(cr4 & ~X86_CR4_VMXE);

    PR_INFO("VMXOFF executed on CPU %d\n", smp_processor_id());
}