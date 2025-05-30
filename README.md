# Get Init System

<a id="markdown-header" name="header"></a>


## If you are in a hurry
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

## Now for the one who are not in a hurry

### The method of testing `/proc/1/exe` and `/proc/1/comm` is very reliable for most Linux distributions, but there are some restrictions and special cases in which this method does not intervene.

`/proc/1/exe`: This is a symbolic link to the executable file of the process with PID 1, i.e. the init system. It shows directly to the binary file (e.g. /lib/systemd/systemd for systemd or /sbin/init for sysvinit). This is a precise method as the path is clear.

`/proc/1/comm`: Returns the name of the process of PID 1, as it is registered in the process management of the kernel (e.g. systemd, init, runit). This is often a good indicator for the init system.

The /proc file system is a standard in Linux, based on the POSIX-like kernel interface, and is supported by virtually all modern Linux distributions, as it is part of the Linux kernel.

### The method works reliably on most standard Linux distributions, including:

- In Systemd distributions `Debian`, `Ubuntu`, `Fedora`, `Arch Linux`, etc. (with `Systemd`, `sysvinit`, `openrc`, etc.).
- Non-system distributions such as `Devuan`, `Void Linux`, `Artix Linux`, `Alpine Linux`.
- Systems with common init systems such as `sysvinit`, `openrc`, `runit`, `s6` or `dinit`.


### However, there are scenarios where the examination of /proc/1/exe and /proc/1/comm is not sufficient or fails:

### 1. Container environments (e.g. Docker, Podman, LXC):
In containers, PID 1 is often not the actual init system of the host system, but a process such as /bin/bash, a special container-init process (e.g. tini, dumb-init) or the main application of the container.
- **Example:** In a docker container, `/proc/1/exe` could show to `/bin/myapp`, although the host systemd used.
- **Solution:** In containers you should additionally check whether you are in a container environment (e.g. by `cat /proc/1/cgroup` or the existence of `/.dockerenv`).



### 2. Restricted /proc access:
Access to `/proc/1/exe` or `/proc/1/comm` can be restricted on systems with high security precautions (e.g. with `SELinux`, `AppArmor` or `Namespaces`), which leads to authorization errors.
- **Example:** A script without root rights could get `Permission denied`.
- **Solution:** The script should incorporate bug treatment (e.g. `2>/dev/null`) or use alternative methods such as `ps -p 1`.

### 3. Non-standard kernel or minimalist systems:
Very minimalist Linux systems (e.g. embedded systems or special builds such as `Buildroot`) could have a restricted /proc file system or disable it completely.
However, such systems are rarely and usually specially adapted.
** Solution:** Fallback to other methods such as testing binaries (e.g. `/sbin/init`).

### 4. Exotic or adapted init systems:
Some distributions or setups use customized init systems that fill `/proc/1/comm` with unusual names (e.g. a script as PID 1).
**Example:** A system could use a custom init script that appears in /proc/1/comm as bash.
** Combine test with other indicators such as specific binaries or configuration files (e.g. `/etc/runit` for `runit`).

### 5 Chroot or Namespaced environments:
In a chroot or an isolated environment, `/proc/1/exe` could show a process that does not correspond to the actual host-init system.
** Solution:** Here you should check if the script runs in a chroot (e.g. by comparing `/proc/1/root` with `/`).

### 6. Systems with mixed init systems:
In rare cases (e.g. in transitions or test environments), several init systems could coexist, making detection more difficult.
**Example:** `Systemd` could run as service manager, but another system (e.g. `openrc`) as PID 1.
** Solution:** In this case, additional systemd-specific interfaces such as D-bus or systemctl should be required.

- - - - -

(German Version)

### Die Methode, `/proc/1/exe` und `/proc/1/comm` zu prüfen, ist auf den meisten Linux-Distributionen sehr zuverlässig, aber es gibt einige Einschränkungen und Sonderfälle, in denen diese Methode nicht greift.

`/proc/1/exe`: Dies ist ein symbolischer Link zur ausführbaren Datei des Prozesses mit PID 1, also dem Init-System. Er zeigt direkt auf die Binärdatei (z.B. /lib/systemd/systemd für Systemd oder /sbin/init für sysvinit). Das ist eine präzise Methode, da der Pfad eindeutig ist.

`/proc/1/comm`: Gibt den Namen des Prozesses von PID 1 zurück, wie er in der Prozessverwaltung des Kernels registriert ist (z.B. systemd, init, runit). Dies ist oft ein guter Indikator für das Init-System.

Das /proc-Dateisystem ist ein Standard in Linux, basierend auf dem POSIX-ähnlichen Kernel-Interface, und wird von praktisch allen modernen Linux-Distributionen unterstützt, da es Teil des Linux-Kernels ist.

### Die Methode funktioniert zuverlässig auf den meisten Standard-Linux-Distributionen, einschließlich:

- In Systemd-Distributionen `Debian`, `Ubuntu`, `Fedora`, `Arch Linux`, etc. (mit `Systemd`, `sysvinit`, `openrc`, etc.).
- Nicht-Systemd-Distributionen wie `Devuan`, `Void Linux`, `Artix Linux`, `Alpine Linux`.
- Systeme mit gängigen Init-Systemen wie `sysvinit`, `openrc`, `runit`, `s6` oder `dinit`.

### Es gibt jedoch Szenarien, in denen die Prüfung von /proc/1/exe und /proc/1/comm nicht ausreicht oder fehlschlägt:

### 1. Container-Umgebungen (z.B. Docker, Podman, LXC):  
In Containern ist PID 1 oft nicht das tatsächliche Init-System des Host-Systems, sondern ein Prozess wie /bin/bash, ein spezieller Container-Init-Prozess (z.B. tini, dumb-init) oder die Hauptanwendung des Containers.  
- **Beispiel:** In einem Docker-Container könnte `/proc/1/exe` auf `/bin/myapp` zeigen, obwohl der Host Systemd verwendet.  
- **Lösung:** In Containern sollte man zusätzlich prüfen, ob man sich in einer Container-Umgebung befindet (z.B. durch `cat /proc/1/cgroup` oder die Existenz von `/.dockerenv`).

### 2. Eingeschränkte /proc-Zugriffe:  
Auf Systemen mit hohen Sicherheitsvorkehrungen (z.B. mit `SELinux`, `AppArmor` oder `Namespaces`) kann der Zugriff auf `/proc/1/exe` oder `/proc/1/comm` eingeschränkt sein, was zu Berechtigungsfehlern führt.  
- **Beispiel:** Ein Skript ohne Root-Rechte könnte `Permission denied` erhalten.  
- **Lösung:** Das Skript sollte Fehlerbehandlung einbauen (z.B. `2>/dev/null`) oder alternative Methoden wie `ps -p 1` verwenden.

### 3. Nicht-Standard-Kernel oder minimalistische Systeme:  
Sehr minimalistische Linux-Systeme (z.B. eingebettete Systeme oder spezielle Builds wie `Buildroot`) könnten ein eingeschränktes /proc-Dateisystem haben oder es ganz deaktivieren.  
Solche Systeme sind jedoch selten und meist speziell angepasst.  
**Lösung:** Fallback auf andere Methoden wie die Prüfung von Binaries (z.B. `/sbin/init`).

### 4.  Exotische oder angepasste Init-Systeme:  
Manche Distributionen oder Setups verwenden angepasste Init-Systeme, die `/proc/1/comm` mit ungewöhnlichen Namen füllen (z.B. ein Skript als PID 1).  
**Beispiel:** Ein System könnte ein benutzerdefiniertes Init-Skript verwenden, das in /proc/1/comm als bash erscheint.  
**Lösung:** Prüfung mit anderen Indikatoren kombinieren, wie z.B. spezifischen Binaries oder Konfigurationsdateien (z.B. `/etc/runit` für `runit`).

### 5. Chroot- oder Namespaced-Umgebungen:  
In einem chroot oder einer isolierten Umgebung könnte `/proc/1/exe` auf einen Prozess zeigen, der nicht dem tatsächlichen Host-Init-System entspricht.  
**Lösung:** Hier müsste man prüfen ob das Skript in einem chroot läuft (z.B. durch Vergleich von `/proc/1/root` mit `/`).

### 6. Systeme mit gemischten Init-Systemen:  
In seltenen Fällen (z.B. bei Übergängen oder Testumgebungen) könnten mehrere Init-Systeme koexistieren, was die Erkennung erschwert.  
**Beispiel:** `Systemd` könnte als Dienst-Manager laufen, aber ein anderes System (z.B. `openrc`) als PID 1.  
**Lösung:** Hier müsste man zusätzliche Systemd-spezifische Schnittstellen wie D-Bus oder systemctl.  

#### Beispielaufruf :
```sh
# Das aufrufen des scripts `get_init_system` stellt ihnen die Functionen `get_init_system` und `get_service_manager` und zusätzlich die Konstanten `INIT_SYSTEM` und `SERVICE_MANAGER` zur Verfügung

./get_init_system

init_system=$(get_init_system)
service_manager=$(get_service_manager)

# und / oder

init_system=${INIT_SYSTEM}
service_manager=${SERVICE_MANAGER}
```