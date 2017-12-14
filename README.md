# ReClassUseExistingProcessHandlePlugin

This is a plugin for ReClass 2016

Official GitHub: https://github.com/dude719/ReClassEx

However, this project doesn't seems to be maintained anymore, you might want to use ReClass.NET instead: https://github.com/KN4CK3R/ReClass.NET

In short, when enabled in ReClass, this plugin first looks if the ReClass process has a process handle to the target process and if it has one, it uses it directly instead of calling OpenProcess.
This is useful since many bypasses allow to have a usable process handle.
If you do not have a bypass that allow you to do that, you can simply use Process Hacker as admin so it launches its kernel module, you can set one of the process handles of the Windows system processes as inheritable and spawn ReClass process as a child process inheriting the handle.

PS: I am having lots of problems with ReClass, sometimes the x64 version doesn't find some processes, then the x86 version apparently cannot load plugins, it's a bit messy.
If you figure what's wrong with all this, or if you have a stable version please contact me.
