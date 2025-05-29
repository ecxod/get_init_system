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
        SM1["Systemd prüfen"]:::start
        SM2["Container-Umgebung prüfen"]:::process
        SM3["Alternativen prüfen"]:::process

        SM1 --> SM2
        SM2 --> SM3
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

    PROC3 --|Ja|--> SYSTEMD
    PROC4 --|init|--> INIT
    PROC4 --|Systemd|--> SYSTEMD
    PROC4 --|SysVinit|--> SYSVINIT
    PROC4 --|OpenRC|--> OPENRC
    PROC4 --|Runit|--> RUNIT
    PROC4 --|S6|--> S6
    PROC4 --|Dinit|--> DINIT
    PROC4 --|Unbekannt|--> UNKNOWN

    PROC1 --|Yes|--> UNKNOWN


    
    PROC2 --|Ja|--> CONTAINER

    
    

    

    



    ```