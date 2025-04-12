/*
Just an Experimental x86_64 hypervisor.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include "tinyvmm.h"
#include "vmx.h"


static int __init tinyVmm_init (void)
{
    PR_INFO("%s < ====  Initilizing TinyVmm  ==== >\n", __func__);
    /* Check for Vt-x support */
    if (!cpu_has_vmx()) {
        PR_ERR("%s VMX not supported on this machine\n", __func__);
        return -ENODEV;
    }
    // Enable VMX on all CPUs
    PR_INFO("%s Enabling VMX on all CPUs\n", __func__);
    on_each_cpu(vmx_enable_on_cpu, NULL, 1);

    return 0;
}

void __exit tinyVmm_exit (void)
{
    PR_INFO("%s < ==== Exiting TinyVmm ==== >\n", __func__);
    on_each_cpu(vmx_disable_on_cpu, NULL, 1);
    return;
}


module_init(tinyVmm_init);
module_exit(tinyVmm_exit);

MODULE_LICENSE("GPLv2");
MODULE_DESCRIPTION("light-weight Hypervisor for x86 platforms");