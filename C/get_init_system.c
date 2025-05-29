#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAX_PATH 256
#define MAX_LINE 256

/* Debugging-Funktion */
void debug(const char *msg) {
    if (getenv("DEBUG") && strcmp(getenv("DEBUG"), "1") == 0) {
        fprintf(stderr, "[DEBUG] %s\n", msg);
    }
}

/* Prüft, ob /proc/1 existiert */
const char *check_proc_1(void) {
    struct stat st;
    if (stat("/proc/1", &st) == 0 && S_ISDIR(st.st_mode)) {
        debug("/proc/1 exists");
        return "";
    }
    debug("/proc/1 not found");
    return "none";
}

/* Prüft, ob es sich um eine Container-Umgebung handelt */
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

/* Prüft, ob systemd als PID 1 läuft */
const char *check_systemd_procone(void) {
    FILE *fp;
    char comm[MAX_LINE];
    char exe[MAX_PATH];
    fp = fopen("/proc/1/comm", "r");
    if (fp) {
        if (fgets(comm, MAX_LINE, fp) && strncmp(comm, "systemd", 7) == 0) {
            fclose(fp);
            debug("systemd detected as PID 1");
            return "systemd";
        }
        fclose(fp);
    }
    /* POSIX-konforme Alternative zu readlink */
    fp = popen("ls -l /proc/1/exe 2>/dev/null | awk '{print $NF}'", "r");
    if (fp) {
        if (fgets(exe, MAX_PATH, fp) && strstr(exe, "/lib/systemd/systemd")) {
            pclose(fp);
            debug("systemd detected via /proc/1/exe");
            return "systemd";
        }
        pclose(fp);
    }
    debug("No systemd detected in PID 1");
    return "";
}

/* Prüft, ob systemd als Service-Manager aktiv ist */
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

/* Ermittelt den Service-Manager */
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

/* Ermittelt das Init-System */
const char *get_init_system(void) {
    const char *result = check_proc_1();
    if (strlen(result) > 0) return result;

    result = check_docker_env();
    if (strlen(result) > 0) return result;

    result = check_systemd_procone();
    if (strlen(result) > 0) return result;

    FILE *fp;
    char comm[MAX_LINE];
    fp = fopen("/proc/1/comm", "r");
    if (fp) {
        if (fgets(comm, MAX_LINE, fp)) {
            comm[strcspn(comm, "\n")] = 0; /* Entferne Newline */
            debug("init_comm: %s", comm);
            if (strcmp(comm, "init") == 0) {
                fclose(fp);
                if (access("/sbin/init", X_OK) == 0) {
                    char exe[MAX_PATH];
                    fp = popen("ls -l /sbin/init 2>/dev/null | awk '{print $NF}'", "r");
                    if (fp && fgets(exe, MAX_PATH, fp)) {
                        exe[strcspn(exe, "\n")] = 0;
                        if (strcmp(exe, "/lib/sysvinit/init") == 0) {
                            pclose(fp);
                            debug("sysvinit detected");
                            return "sysvinit";
                        }
                        pclose(fp);
                    }
                    fp = popen("grep -q Upstart /sbin/init 2>/dev/null && echo upstart", "r");
                    if (fp && fgets(comm, MAX_LINE, fp) && strcmp(comm, "upstart\n") == 0) {
                        pclose(fp);
                        debug("upstart detected");
                        return "upstart";
                    }
                    pclose(fp);
                    debug("Unknown init system for /sbin/init");
                    return "unknown";
                }
                debug("No /sbin/init found");
                return "unknown";
            }
            if (strcmp(comm, "runit") == 0) { debug("runit detected"); return "runit"; }
            if (strcmp(comm, "openrc-init") == 0) { debug("openrc detected"); return "openrc"; }
            if (strcmp(comm, "s6-svscan") == 0) { debug("s6 detected"); return "s6"; }
            if (strcmp(comm, "dinit") == 0) { debug("dinit detected"); return "dinit"; }
        }
        fclose(fp);
    }
    if (access("/sbin/openrc-init", X_OK) == 0) {
        debug("openrc detected via /sbin/openrc-init");
        return "openrc";
    }
    if (access("/usr/bin/runit", X_OK) == 0) {
        debug("runit detected via /usr/bin/runit");
        return "runit";
    }
    if (access("/sbin/dinit", X_OK) == 0) {
        debug("dinit detected via /sbin/dinit");
        return "dinit";
    }
    debug("No known init system detected");
    return "unknown";
}

int main(void) {
    const char *init_system = get_init_system();
    const char *service_manager = get_service_manager();
    printf("%s\n%s\n", init_system, service_manager);
    return 0;
}