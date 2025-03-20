# root_exec kernel module

A kernel module (for use in CTFs) that will execute arbitrary commands

1. use `make` to build a root_exec.ko
2. then `sudo insmod root_exec.ko` to install the module (obviously you need permissions to do this)
3. finally, the command will be run when you cat /proc/root_exec

By default, the root_exec.c module will copy /bin/sh to tmp, and make it suid so you can use `sh -p` to jump to root.

Credits:

- deepseek (after a long battle with various hallucinations and dire warnings) helped me write this
- it was built for the challenge 'Avengers Hub' in [TryHackMe](https://tryhackme.com)'s Hackfinity CTF, March 2025
