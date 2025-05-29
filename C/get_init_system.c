#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Hilfsfunktion zum Lesen einer Zeile aus einer Datei
static char* read_line(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return NULL;
    
    size_t size = 128;
    char* buffer = malloc(size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t len = 0;
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (len >= size - 1) {
            size *= 2;
            char* temp = realloc(buffer, size);
            if (!temp) {
                free(buffer);
                fclose(file);
                return NULL;
            }
            buffer = temp;
        }
        buffer[len++] = c;
    }
    
    if (len == 0) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[len] = '\0';
    fclose(file);
    return buffer;
}

// Hilfsfunktion zum Lesen eines Symlinks
static char* read_symlink(const char* path) {
    static char target[1024];
    ssize_t len = readlink(path, target, sizeof(target) - 1);
    if (len == -1) return NULL;
    target[len] = '\0';
    return strdup(target);
}

// Prüft, ob eine Datei existiert und ausführbar ist
static int is_executable(const char* path) {
    struct stat sb;
    return (stat(path, &sb) == 0 && S_ISREG(sb.st_mode) && 
           (access(path, X_OK) == 0));
}

// Hauptfunktion zur Erkennung des Init-Systems
const char* get_init_system() {
    // Prüfe /proc Verzeichnis
    if (access("/proc/1", F_OK) != 0) {
        return "unknown (no /proc)";
    }

    // Prüfe auf Systemd als PID 1
    char* systemd_path = read_symlink("/proc/1/exe");
    char* comm = read_line("/proc/1/comm");
    if (systemd_path && strcmp(systemd_path, "/lib/systemd/systemd") == 0) {
        free(systemd_path);
        free(comm);
        return "false";
    }

    // Prüfe Container-Umgebung
    if (access("/.dockerenv", F_OK) == 0) {
        free(systemd_path);
        free(comm);
        return "container";
    }

    FILE* fp = fopen("/proc/1/cgroup", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "docker") || strstr(line, "lxc") ||
                strstr(line, "container")) {
                fclose(fp);
                free(systemd_path);
                free(comm);
                return "container";
            }
        }
        fclose(fp);
    }

    // Identifiziere Init-System basierend auf dem Comm-Namen
    if (comm && strcmp(comm, "init") == 0) {
        if (is_executable("/sbin/init")) {
            char* init_link = read_symlink("/sbin/init");
            if (init_link && strcmp(init_link, "/lib/sysvinit/init") == 0) {
                free(init_link);
                free(systemd_path);
                free(comm);
                return "sysvinit";
            }
            free(init_link);
        }
        free(systemd_path);
        free(comm);
        return "unknown";
    } else if (comm && strcmp(comm, "runit") == 0) {
        free(systemd_path);
        free(comm);
        return "runit";
    } else if (comm && strcmp(comm, "openrc-init") == 0) {
        free(systemd_path);
        free(comm);
        return "openrc";
    } else if (comm && strcmp(comm, "s6-svscan") == 0) {
        free(systemd_path);
        free(comm);
        return "s6";
    } else if (comm && strcmp(comm, "dinit") == 0) {
        free(systemd_path);
        free(comm);
        return "dinit";
    }

    // Fallback-Prüfungen
    if (is_executable("/sbin/openrc-init")) {
        free(systemd_path);
        free(comm);
        return "openrc";
    } else if (is_executable("/usr/bin/runit")) {
        free(systemd_path);
        free(comm);
        return "runit";
    } else if (is_executable("/sbin/dinit")) {
        free(systemd_path);
        free(comm);
        return "dinit";
    } else if (is_executable("/sbin/init") && 
               strcmp(read_symlink("/sbin/init"), "/lib/sysvinit/init") == 0) {
        free(systemd_path);
        free(comm);
        return "sysvinit";
    }

    free(systemd_path);
    free(comm);
    return "unknown";
}

// Hauptfunktion zur Erkennung des Service-Managers
const char* get_service_manager() {
    // Prüfe /proc Verzeichnis
    if (access("/proc/1", F_OK) != 0) {
        return "unknown (no /proc)";
    }

    // Prüfe auf Systemd als Service-Manager
    struct stat sb;
    if (stat("/run/systemd/system", &sb) == 0) {
        return "false";
    }

    // Prüfe Container-Umgebung
    if (access("/.dockerenv", F_OK) == 0) {
        return "container";
    }

    FILE* fp = fopen("/proc/1/cgroup", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "docker") || strstr(line, "lxc") ||
                strstr(line, "container")) {
                fclose(fp);
                return "container";
            }
        }
        fclose(fp);
    }

    // Prüfe verschiedene Service-Manager
    if (is_executable("/sbin/rc-service") && 
        system("/sbin/rc-service --version >/dev/null 2>&1") == 0) {
        return "openrc";
    } else if (access("/etc/init.d", F_OK) == 0 &&
               is_executable("/sbin/service")) {
        return "sysvinit";
    } else if (is_executable("/usr/bin/sv") && 
               system("/usr/bin/sv status >/dev/null 2>&1") == 0) {
        return "runit";
    } else if (is_executable("/usr/bin/s6-rc")) {
        return "s6";
    } else if (is_executable("/usr/bin/dinitctl")) {
        return "dinit";
    }

    // Fallback-Prüfungen
    if (access("/etc/s6-rc", F_OK) == 0) {
        return "s6";
    } else if (access("/etc/runit", F_OK) == 0) {
        return "runit";
    } else if (access("/etc/dinit.d", F_OK) == 0) {
        return "dinit";
    } else if (access("/etc/init.d", F_OK) == 0) {
        return "sysvinit";
    }

    return "unknown";
}

int main() {
    const char* init_system = get_init_system();
    const char* service_manager = get_service_manager();

    printf("Init System: %s\n", init_system);
    printf("Service Manager: %s\n", service_manager);

    return 0;
}