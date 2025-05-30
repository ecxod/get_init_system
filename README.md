# Get Init System

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