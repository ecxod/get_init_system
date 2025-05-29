#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

#define MAX_PATH 256
#define MAX_LINE 256

/* Debugging function with format string support */
void debug(const char *fmt, ...) {
    if (getenv("DEBUG") && strcmp(getenv("DEBUG"), "1") == 0) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "[DEBUG] ");
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}

/* Checks whether this is a container environment */
const char *check_docker_env(void) {
    FILE *fp;
    char line[MAX_LINE];
    if (access("/.dockerenv", F_OK) == 0 || access("/run/.containerenv", F_OK) == 0) {
        debug("Container environment detected");
        return "container";
    }
    fp = fopen("/proc/1/cgroup", "r");
    if (fp) {
        while (fgets(line, MAX_LINE, fp)) {
            if (strstr(line, "docker") || strstr(line, "lxc") || strstr(line, "container")) {
                fclose(fp);
                debug("Container detected via /proc/1/cgroup");
                return "container";
            }
        }
        fclose(fp);
    }
    if (getenv("container")) {
        debug("Container detected via environment variable");
        return "container";
    }
    debug("No container environment detected");
    return "";
}

/* Checks whether systemd is active as a service manager */
const char *systemd_service_manager(void) {
    if (system("systemctl --version >/dev/null 2>&1") == 0 &&
        system("dbus-send --system --dest=org.freedesktop.systemd1 "
               "--print-reply /org/freedesktop/systemd1 "
               "org.freedesktop.DBus.Peer.Ping >/dev/null 2>&1") == 0) {
        debug("systemd detected as service manager");
        return "systemd";
    }
    debug("No systemd service manager detected");
    return "";
}

/* Determines the Service Manager */
const char *get_service_manager(void) {
    const char *result = check_docker_env();
    if (strlen(result) > 0) return result;

    result = systemd_service_manager();
    if (strlen(result) > 0) return result;

    if (access("/sbin/rc-service", X_OK) == 0 && system("/sbin/rc-service --version >/dev/null 2>&1") == 0) {
        debug("openrc detected as service manager");
        return "openrc";
    }
    if (access("/etc/init.d", F_OK) == 0 && access("/sbin/service", X_OK) == 0) {
        debug("init detected as service manager");
        return "init";
    }
    if (access("/usr/bin/sv", X_OK) == 0 && system("/usr/bin/sv status >/dev/null 2>&1") == 0) {
        debug("runit detected as service manager");
        return "runit";
    }
    if (access("/usr/bin/s6-rc", X_OK) == 0) {
        debug("s6 detected as service manager");
        return "s6";
    }
    if (access("/usr/bin/dinitctl", X_OK) == 0) {
        debug("dinit detected as service manager");
        return "dinit";
    }
    if (access("/etc/s6-rc", F_OK) == 0) {
        debug("s6 detected via /etc/s6-rc");
        return "s6";
    }
    if (access("/etc/runit", F_OK) == 0) {
        debug("runit detected via /etc/runit");
        return "runit";
    }
    if (access("/etc/dinit.d", F_OK) == 0) {
        debug("dinit detected via /etc/dinit.d");
        return "dinit";
    }
    if (access("/etc/init.d", F_OK) == 0) {
        debug("sysvinit detected via /etc/init.d");
        return "sysvinit";
    }
    debug("No service manager detected");
    return "unknown";
}

int main(void) {
    const char *service_manager = get_service_manager();
    printf("%s\n", service_manager);
    return 0;
}