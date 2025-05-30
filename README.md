# Get Init System


<!-- TOC -->
- [1. If you are in a hurry](#1-if-you-are-in-a-hurry)
- [2. Now for the one who are not in a hurry](#2-now-for-the-one-who-are-not-in-a-hurry)
    - [2.1. The method of testing `/proc/1/exe` and `/proc/1/comm`](#21-the-method-of-testing-proc1exe-and-proc1comm)
    - [2.2. Linux Distributions](#22-linux-distributions)
    - [2.3. Scenarios where it does not work](#23-scenarios-where-it-does-not-work)
        - [2.3.1. Container environments (e.g. Docker, Podman, LXC):](#231-container-environments-eg-docker-podman-lxc)
        - [2.3.2 Restricted /proc access:](#232-restricted-proc-access)
        - [2.3.3 Non-standard kernel or minimalist systems:](#233-non-standard-kernel-or-minimalist-systems)
        - [2.3.4. Exotic or adapted init systems:](#234-exotic-or-adapted-init-systems)
        - [2.3.5. Chroot or Namespaced environments:](#235-chroot-or-namespaced-environments)
        - [2.3.6. Systems with mixed init systems:](#236-systems-with-mixed-init-systems)
<!-- /TOC -->



## 1. If you are in a hurry
Please note that you can compile the C script (no root needed) and place the binary in your `/usr/local/bin`, or somewhere else in your PATH. This script recognizes your `Init System`. If you also want to be able to recognize the `Service Manager`, which could be like the Init system, but could also be different (mixed systems), please note that this program executes two commands `get_init_system` and `get_service_manager`. To do this, you must create a link to the main binary in the `/usr/local/bin` folder. The program will recognize that it has been called `get_service_manager` and shows you the `Service Manager`.

```sh
# compile
gcc -o get_init_system get_init_system.c
# move to `/usr/local/bin`
mv get_init_system /usr/local/bin/get_init_system
# create the link
ln -s /usr/local/bin/get_init_system /usr/local/bin/get_service_manager
```

How to use it? : simply call the program
```sh
user@host $ get_init_system
sysvinit
```

or alternatively for the Service Manager
```sh
user@host $ get_service_manager
systemd
```

There is also a debug mode, export the DEBUG variable, and use it normaly
```sh
DEBUG=1 get_init_system
DEBUG=1 get_service_manager
```

If you want to use it in your Docker, copy it there and call it
```sh
docker run -it <container_id> sh
docker cp get_init_system <container_id>:/your/path/get_init_system
docker exec <container_id> ln -s /your/path/get_init_system /your/path/get_service_manager
```

- - - - -

## 2. Now for the one who are not in a hurry

### 2.1. The method of testing `/proc/1/exe` and `/proc/1/comm` 
is very reliable for most Linux distributions, 
but there are some restrictions and special cases in which this method does not intervene.

`/proc/1/exe`: This is a symbolic link to the executable file of the process with PID 1, i.e. the init system. It shows directly to the binary file (e.g. /lib/systemd/systemd for systemd or /sbin/init for sysvinit). This is a precise method as the path is clear.

`/proc/1/comm`: Returns the name of the process of PID 1, as it is registered in the process management of the kernel (e.g. systemd, init, runit). This is often a good indicator for the init system.

The /proc file system is a standard in Linux, based on the POSIX-like kernel interface, and is supported by virtually all modern Linux distributions, as it is part of the Linux kernel.

### 2.2. Linux Distributions
The method works reliably on most standard Linux distributions, including:

- In Systemd distributions `Debian`, `Ubuntu`, `Fedora`, `Arch Linux`, etc. (with `Systemd`, `sysvinit`, `openrc`, etc.).
- Non-system distributions such as `Devuan`, `Void Linux`, `Artix Linux`, `Alpine Linux`.
- Systems with common init systems such as `sysvinit`, `openrc`, `runit`, `s6` or `dinit`.

### 2.3. Scenarios where it does not work
However, there are scenarios where the examination of /proc/1/exe and /proc/1/comm is not sufficient or fails:

#### 2.3.1. Container environments (e.g. Docker, Podman, LXC):
In containers, PID 1 is often not the acStual init system of the host system, but a process such as /bin/bash, a special container-init process (e.g. tini, dumb-init) or the main application of the container.
Example: In a docker container, `/proc/1/exe` could show to `/bin/myapp`, although the host systemd used. In containers you should additionally check whether you are in a container environment (e.g. by `cat /proc/1/cgroup` or the existence of `/.dockerenv`).

#### 2.3.2 Restricted /proc access:
Access to `/proc/1/exe` or `/proc/1/comm` can be restricted on systems with high security precautions (e.g. with `SELinux`, `AppArmor` or `Namespaces`), which leads to authorization errors.
For example a script without root rights could get `Permission denied`.
In this case the script should incorporate bug treatment (e.g. `2>/dev/null`) or use alternative methods such as `ps -p 1`.

#### 2.3.3 Non-standard kernel or minimalist systems:
Very minimalist Linux systems (e.g. embedded systems or special builds such as `Buildroot`) could have a restricted /proc file system or disable it completely.
However, such systems are rarely and usually specially adapted.
In this case fallback to other methods such as testing binaries (e.g. `/sbin/init`).

#### 2.3.4. Exotic or adapted init systems:
Some distributions or setups use customized init systems that fill `/proc/1/comm` with unusual names (e.g. a script as PID 1).
A system could use a custom init script that appears in /proc/1/comm as bash. Combine test with other indicators such as specific binaries or configuration files (e.g. `/etc/runit` for `runit`).

#### 2.3.5. Chroot or Namespaced environments:
In a chroot or an isolated environment, `/proc/1/exe` could show a process that does not correspond to the actual host-init system.
Here you should check if the script runs in a chroot (e.g. by comparing `/proc/1/root` with `/`).

#### 2.3.6. Systems with mixed init systems:
In rare cases (e.g. in transitions or test environments), several init systems could coexist, making detection more difficult.
`Systemd` could run as service manager, but another system (e.g. `openrc`) as PID 1. In this case, additional systemd-specific interfaces such as D-bus or systemctl should be required.

