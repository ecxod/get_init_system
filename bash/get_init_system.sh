#!/bin/sh
# POSIX-konformes Skript zur Erkennung von Init- und Dienst-Manager

check_proc_1() {
    # test if we can see /proc, else we return unknown
    if [ -d /proc/1 ]
    then
        echo ""
        return
    else
        echo "unknown"
        return
    fi
}

check_docker_env() {
    # test if we are in a Container-Enviroment
    if  [ -f /.dockerenv ] \
        || \
        grep 'docker\|lxc\|container' /proc/1/cgroup >/dev/null 2>&1
    then
        echo "container"
        return
    else;
        echo ""
        return
    fi
}

check_systed_procone() {
    if [ "$(readlink /proc/1/exe 2>/dev/null)" = "/lib/systemd/systemd" ] || \
       [ "$(cat /proc/1/comm 2>/dev/null)" = "systemd" ]
    then
        echo "systemd"
        return
    else
        echo ""
        return
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
        return
    elif
    then
        echo ""
        return
    fi
}



get_service_manager() {
    result=$(check_proc_1)
    if [ -z ${result} ]; then 
        result=$(check_proc_1)
        if [ -z ${result} ]; then 
            result=$(check_docker_env)
            if [ -z ${result} ]; then 
                result=$(systemd_service_manager)
                if [ -z ${result} ]
                    # Identifiziere alternative Dienst-Manager
                    if [ -x /sbin/rc-service ] && /sbin/rc-service --version >/dev/null 2>&1; then result="openrc";
                    elif [ -d /etc/init.d ] && [ -x /sbin/service ]; then result="init";
                    elif [ -x /usr/bin/sv ] && /usr/bin/sv status >/dev/null 2>&1; then result="runit";
                    elif [ -x /usr/bin/s6-rc ]; then result="s6";
                    elif [ -x /usr/bin/dinitctl ]; then result="dinit";
                    else
                        # Fallback: Prüfe bekannte Konfigurationsverzeichnisse
                        if [ -d /etc/s6-rc ]; then result="s6";
                        elif [ -d /etc/runit ]; then result="runit";
                        elif [ -d /etc/dinit.d ]; then result="dinit";
                        elif [ -d /etc/init.d ]; then result="sysvinit";
                        else result="unknown";
                        fi
                    fi
                fi
            fi
        fi
    fi
    echo ${result}
}

get_init_system() {
    result=$(check_proc_1)
    if [ -z ${result} ]; then 
        result=$(check_proc_1)
        if [ -z ${result} ]; then  
            result=$(check_docker_env)
            if [ -z ${result} ]; then  
                result=$(check_systed_procone)
                if [ -z ${result} ]
                    # Identifiziere Init-System
                    #init_exe=$(readlink /proc/1/exe 2>/dev/null)
                    init_comm=$(cat /proc/1/comm 2>/dev/null)

                    case "$init_comm" in
                        "init")
                            if [ -x /sbin/init ] && [ "$(readlink /sbin/init)" = "/lib/sysvinit/init" ]; then result="sysvinit";
                            elif [ -x /sbin/init ] && grep -q "Upstart" /sbin/init 2>/dev/null; then result="upstart";
                            elif [ -x /sbin/init ] && grep -q "Init" /sbin/init 2>/dev/null; then result="init";
                            else result="unknown";
                            fi
                            ;;
                        "runit") result="runit" ;;
                        "openrc-init") result="openrc" ;;
                        "s6-svscan") result="s6" ;;
                        "dinit") result="dinit" ;;
                        *)
                            if [ -x /sbin/openrc-init ]; then result="openrc";
                            elif [ -x /usr/bin/runit ]; then result="runit";
                            elif [ -x /sbin/dinit ]; then result="dinit";
                            elif [ -x /sbin/init ] && [ "$(readlink /sbin/init)" = "/lib/sysvinit/init" ]; then result="sysvinit";
                            else result="unknown";
                            fi
                            ;;
                    esac
                fi
            fi
        fi
    fi
    echo ${result}
}

INIT_SYSTEM=$(get_init_system)
SERVICE_MANAGER=$(get_service_manager)

exit 0