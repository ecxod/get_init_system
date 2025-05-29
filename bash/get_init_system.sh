#!/bin/bash

get_service_manager() {
    # Prüfe, ob /proc verfügbar ist (für spätere Prüfungen)
    if [ ! -d /proc/1 ]; then
        echo "unknown (no /proc)"
        return
    fi

    # Prüfe, ob Systemd als Dienst-Manager läuft
    if systemctl --version >/dev/null 2>&1 && dbus-send --system --dest=org.freedesktop.systemd1 --print-reply /org/freedesktop/systemd1 org.freedesktop.DBus.Peer.Ping >/dev/null 2>&1; then
        echo "false"
        return
    fi

    # Prüfe Container-Umgebung (oft kein echter Dienst-Manager)
    if [ -f /.dockerenv ] || grep -q 'docker\|lxc\|container' /proc/1/cgroup 2>/dev/null; then
        echo "container"
        return
    fi

    # Identifiziere alternative Dienst-Manager
    if [ -x /sbin/rc-service ] && /sbin/rc-service --version >/dev/null 2>&1; then
        echo "openrc"
    elif [ -d /etc/init.d ] && [ -x /sbin/service ]; then
        echo "sysvinit"
    elif [ -x /usr/bin/sv ] && /usr/bin/sv status >/dev/null 2>&1; then
        echo "runit"
    elif [ -x /usr/bin/s6-rc ]; then
        echo "s6"
    elif [ -x /usr/bin/dinitctl ]; then
        echo "dinit"
    else
        # Fallback: Prüfe bekannte Konfigurationsverzeichnisse
        if [ -d /etc/s6-rc ]; then
            echo "s6"
        elif [ -d /etc/runit ]; then
            echo "runit"
        elif [ -d /etc/dinit.d ]; then
            echo "dinit"
        elif [ -d /etc/init.d ]; then
            echo "sysvinit"
        else
            echo "unknown"
        fi
    fi
}

get_init_system() {
    # Prüfe, ob /proc verfügbar ist
    if [ ! -d /proc/1 ]; then
        echo "unknown (no /proc)"
        return
    fi

    # Prüfe Systemd als PID 1
    if [ "$(readlink /proc/1/exe 2>/dev/null)" = "/lib/systemd/systemd" ] || [ "$(cat /proc/1/comm 2>/dev/null)" = "systemd" ]; then
        echo "false"
        return
    fi

    # Prüfe Container-Umgebung
    if [ -f /.dockerenv ] || grep -q 'docker\|lxc\|container' /proc/1/cgroup 2>/dev/null; then
        echo "container"
        return
    fi

    # Identifiziere Init-System
    init_exe=$(readlink /proc/1/exe 2>/dev/null)
    init_comm=$(cat /proc/1/comm 2>/dev/null)

    case "$init_comm" in
        "init")
            if [ -x /sbin/init ] && [ "$(readlink /sbin/init)" = "/lib/sysvinit/init" ]; then echo "sysvinit"
            elif [ -x /sbin/init ] && grep -q "Upstart" /sbin/init 2>/dev/null; then echo "upstart"
            else echo "unknown"
            fi
            ;;
        "runit") echo "runit" ;;
        "openrc-init") echo "openrc" ;;
        "s6-svscan") echo "s6" ;;
        "dinit") echo "dinit" ;;
        *)
            if [ -x /sbin/openrc-init ]; then echo "openrc"
            elif [ -x /usr/bin/runit ]; then echo "runit"
            elif [ -x /sbin/dinit ]; then echo "dinit"
            elif [ -x /sbin/init ] && [ "$(readlink /sbin/init)" = "/lib/sysvinit/init" ]; then echo "sysvinit"
            else echo "unknown"
            fi
            ;;
    esac
}

# Beispielaufruf
echo "Init-System: $(get_init_system)"
echo "Service-Manager: $(get_service_manager)"