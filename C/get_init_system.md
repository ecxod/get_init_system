# Get Init System

```mmd
flowchart TD
    %% Styling
    classDef start fill:#4CAF50,stroke:#45a049,color:#fff
    classDef process fill:#2196F3,stroke:#1976D2,color:#fff
    classDef decision fill:#FFC107,stroke:#FFA000,color:#000
    classDef result fill:#9C27B0,stroke:#7B1FA2,color:#fff
    classDef container fill:#FF5722,stroke:#E64A19,color:#fff

    subgraph Service1["Init-System Detection"]
        PROC1{"/proc/1 present?"}:::start
        PROC2{"Container-Env?"}:::decision
        PROC3{"Systemd?"}:::decision
        PROC4["Analize /proc/1/com"]:::process
        
        PROC1 --|No|--> PROC2
        PROC2 --|No|--> PROC3
        PROC3 --|No|--> PROC4
    end
    
    subgraph Service2["Service-Manager Detection"]
        SYSM1{"/proc/1 present?"}:::start
        SYSM2{"Container-Env?"}:::decision
        SYSM3{"Systemd?"}:::decision
        SYSM4["Alternativen prüfen"]:::process

        SYSM1 --|No|--> SYSM2
        SYSM2 --|No|--> SYSM3
        SYSM3 --|No|--> SYSM4
    end
    
    subgraph Ergebnisse["Ergebnisse"]
        CONTAINER["Container"]:::result
        SYSTEMD["Systemd"]:::result
        INIT["Init"]:::result
        SYSVINIT["SysVinit"]:::result
        OPENRC["OpenRC"]:::result
        RUNIT["Runit"]:::result
        S6["S6"]:::result
        DINIT["Dinit"]:::result
        UNKNOWN["Unknown"]:::result
    end

    INIT_SYSTEM

    PROC1 --|Yes|---> UNKNOWN
    PROC2 --|Yes|---> CONTAINER
    PROC3 --|Yes|---> SYSTEMD

    PROC4 --|init|---> INIT
    PROC4 --|Systemd|---> SYSTEMD
    PROC4 --|SysVinit|--> SYSVINIT
    PROC4 --|OpenRC|--> OPENRC
    PROC4 --|Runit|--> RUNIT
    PROC4 --|S6|--> S6
    PROC4 --|Dinit|--> DINIT
    PROC4 --|Unbekannt|--> UNKNOWN


    SYSM1 --|Yes|---> UNKNOWN
    SYSM2 --|Yes|---> CONTAINER
    SYSM3 --|Yes|---> SYSTEMD

    SYSM4 --|init|---> INIT
    SYSM4 --|Systemd|--> SYSTEMD
    SYSM4 --|SysVinit|--> SYSVINIT
    SYSM4 --|OpenRC|--> OPENRC
    SYSM4 --|Runit|--> RUNIT
    SYSM4 --|S6|--> S6
    SYSM4 --|Dinit|--> DINIT
    SYSM4 --|Unbekannt|--> UNKNOWN



    
    

    

    



    ```