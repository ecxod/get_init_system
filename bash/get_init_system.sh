#!/bin/sh
# POSIX-konformes Skript zur Erkennung von Init- und Dienst-Manager
# get_init_system: Ermittelt das Init-System des laufenden Linux-Systems
# Rückgabewerte: systemd, sysvinit, upstart, openrc, runit, s6, dinit, container, unknown

if [ ! -d /proc ]; then
    echo "unknown"
    exit 0
fi

check_proc_1() {
    # test if we can see /proc, else we return unknown
    if [ -d /proc/1 ]
    then
        echo ""
    else
        echo "unknown"
    fi
}

check_docker_env() {
    if [ -f /.dockerenv ] || [ -f /run/.containerenv ] ||
       grep -qE 'docker|lxc|container' /proc/1/cgroup 2>/dev/null ||
       [ -n "$container" ]; then
        echo "container"
    else
        echo ""
    fi
}

check_systed_procone() {
    if [ "$(readlink_alternative /proc/1/exe 2>/dev/null)" = "/lib/systemd/systemd" ] || \
       [ "$(cat /proc/1/comm 2>/dev/null)" = "systemd" ]
    then
        echo "systemd"
    else
        echo ""
    fi
}

systemd_service_manager() {
    # test if Systemd runs as service manager
    if  systemctl   --version >/dev/null 2>&1 \
        && \
        dbus-send   --system \
                    --dest=org.freedesktop.systemd1 \
                    --print-reply /org/freedesktop/systemd1 org.freedesktop.DBus.Peer.Ping >/dev/null 2>&1
    then 
        echo "systemd"
    else
        echo ""
    fi
}

readlink_alternative() {
    ls -l "$1" 2>/dev/null | awk '{print $NF}'
}


get_service_manager() {
    # Prüfe zuerst Container-Umgebung
    result=$(check_docker_env)
    if [ -n "$result" ]; then
        echo "$result"
        return
    fi

    # Prüfe systemd als Service-Manager
    result=$(systemd_service_manager)
    if [ -n "$result" ]; then
        echo "$result"
        return
    fi

    # Identifiziere alternative Dienst-Manager
    if [ -x /sbin/rc-service ] && /sbin/rc-service --version >/dev/null 2>&1; then
        echo "openrc"
    elif [ -d /etc/init.d ] && [ -x /sbin/service ]; then
        echo "init"
    elif [ -x /usr/bin/sv ] && /usr/bin/sv status >/dev/null 2>&1; then
        echo "runit"
    elif [ -x /usr/bin/s6-rc ]; then
        echo "s6"
    elif [ -x /usr/bin/dinitctl ]; then
        echo "dinit"
    else
        # Fallback: Prüfe Konfigurationsverzeichnisse
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
    # Prüfe zuerst Container-Umgebung
    result=$(check_docker_env)
    if [ -n "$result" ]; then
        echo "$result"
        return
    fi

    # Prüfe systemd
    result=$(check_systed_procone)
    if [ -n "$result" ]; then
        echo "$result"
        return
    fi

    # Prüfe /proc/1/comm
    init_comm=$(cat /proc/1/comm 2>/dev/null)
    case "$init_comm" in
        "init")
            if [ -x /sbin/init ]; then
                if [ "$(readlink_alternative /sbin/init)" = "/lib/sysvinit/init" ]; then
                    echo "sysvinit"
                elif grep -q "Upstart" /sbin/init 2>/dev/null; then
                    echo "upstart"
                else
                    echo "unknown"
                fi
            else
                echo "unknown"
            fi
            ;;
        "runit") echo "runit" ;;
        "openrc-init") echo "openrc" ;;
        "s6-svscan") echo "s6" ;;
        "dinit") echo "dinit" ;;
        *)
            if [ -x /sbin/openrc-init ]; then
                echo "openrc"
            elif [ -x /usr/bin/runit ]; then
                echo "runit"
            elif [ -x /sbin/dinit ]; then
                echo "dinit"
            else
                echo "unknown"
            fi
            ;;
    esac
}

INIT_SYSTEM=$(get_init_system)
SERVICE_MANAGER=$(get_service_manager)

exit 0