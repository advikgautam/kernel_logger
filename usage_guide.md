# Quick Start Guide: Downloading and Running the Kernel Module

This guide will walk you through downloading the Kernel State Logger project from GitHub, compiling it, and safely running it inside your Linux kernel.

## Step 1: Install Prerequisites

Before you can compile kernel code, your Linux machine needs Git (to download the code), a compiler, and the headers for your specific kernel.

**If you are on Arch Linux or CachyOS:**

Open your terminal and run:

```bash
sudo pacman -Syu git base-devel linux-headers
```

*(Note: If you are running a custom CachyOS kernel, you may need to install `linux-cachyos-headers` instead of `linux-headers`).*

**If your teammates are on Ubuntu or Debian VMs:**

```bash
sudo apt update
sudo apt install git build-essential linux-headers-$(uname -r)
```

## Step 2: Download the Repository

Use Git to pull the project files from GitHub to your local machine.

```bash
# Clone the repository (Replace YOUR-USERNAME with the actual GitHub username)
git clone https://github.com/YOUR-USERNAME/kernel_logger.git

# Move into the project directory
cd kernel_logger
```

**Why?** `git clone` downloads the code, and `cd` (change directory) moves your terminal inside the downloaded folder so you can interact with the files.

## Step 3: Compile the Source Code

Now that you have the `logger.c` and `Makefile` files, you need to translate the human-readable C code into a compiled Kernel Object (`.ko`) that the OS can understand.

```bash
make
```

*(Note for CachyOS users: If the standard `make` command fails with a GCC compiler error, force the system to use the LLVM toolchain by running `make LLVM=1` instead).*

If successful, type `ls` in your terminal. You should now see a new file called `logger.ko`. This is your compiled kernel module!

## Step 4: Inject the Module into the Kernel

It is time to load the code into Ring 0 (Kernel Space). Because modifying the kernel is highly restricted, you must use `sudo` to run this as the root administrator.

```bash
sudo insmod logger.ko
```

**Why?** `insmod` stands for "Insert Module." The exact millisecond you hit Enter, the `logger_init()` function inside the C code is executed by the kernel. As of this update, that function now also walks the kernel's live process list and logs every running process.

## Step 5: View the Kernel Logs

Unlike regular programs, kernel modules do not print their text to your standard terminal screen. They write their messages to a hidden, internal kernel log called the ring buffer. We need to read that buffer.

```bash
dmesg | tail -n 20
```

*(We bumped this from `-n 10` to `-n 20` because the process list now adds one log line per running process, so the uptime message can get pushed further up.)*

**What to expect:** You should see output at the bottom of the logs saying something like:

```
LOGGER: Module loaded.
LOGGER: System uptime is 3450 seconds.
LOGGER: --- Active Process List ---
LOGGER: PID: 1 | Name: systemd
LOGGER: PID: 2 | Name: kthreadd
LOGGER: PID: 512 | Name: firefox
LOGGER: --- End of Process List ---
```

## Step 6: Remove the Module

When you are done testing, you must safely remove the module from the kernel to free up memory.

```bash
sudo rmmod logger
```

**Why?** `rmmod` stands for "Remove Module." This triggers the `logger_exit()` function in the C code, safely cleaning up the module.

## Step 7: Verify the Final Output

Check the kernel logs one last time to see the final calculation made by the exit function.

```bash
dmesg | tail -n 5
```

**What to expect:** You should see the final messages showing exactly how long the module was alive inside the kernel:

```
LOGGER: Module unloaded.
LOGGER: Module was active for 42 seconds.
```
