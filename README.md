# Algorand Node for Windows

This repository contains the supporting software for running an Algorand Node under Windows systems, including a native Windows service, companion tools and proper MSI based installer and uninstaller.

> :warning: 32-bit Windows is not supported.

## How to Install

To install Algorand Node for Windows, please download and execute the package you prefer from the **Releases** section on this repository. They follow the **stable** branch and version scheme of the official node.

## Preliminary Information 

Please read the following usage guidelines and advices.

* As they may have different binaries, testnet, betanet, mainnet packages are considered products on their own, so:

    * Installations for different networks and/or versions can co exist in the same system.
    * Upgrades can be performed *only* between packages of the same network node.  e.g: 2.2.0-testnet to 2.3.0-testnet.

* Once the product is installed, do not modify the network it's operating on by changing the configuration file. Use the proper installer. 

* Automatic downgrades are not supported. If you want to install a previous version, uninstall the current version completely first.

### Known bugs :warning:

* Coexisting installations may show only one set of shortcuts.  

* The Node Watch Status window displays ANSI escape codes instead of interpreting them properly. This does not interfere with its functionality.

* Using the _Maintenance_ or _Repair_ Dialog from the installation MSI does not work.  Please use the **Add/Remove Programs** entry in the **System Settings** panel.


## Installation

> :lock: Administrative rights are required to complete the installation.

To launch the installation process just double click the MSI file.  Read and accept the license agreement and choose your target installation directory. We recommend to use the default installation place.

The installer will display a dialog to configure your node with the following options:

* **Port Number** A port where the node server will listen to. You can select zero for autoconfiguration, or a fixed port number. Take caution to choose a port that  does not clash with existing software. Also, you may need to open your firewall for certain range of ports.

* **Public Access** If you select this option, your server will be accessible from the outside.

* **Archival Mode** Enabling Archival will synchronize the entire ledger, greatly increasing disk requirements. 

* **Start service at boot** Self-descriptive, check if you want to start Algorand Node service with your machine. Otherwise, it will be set in "demand" mode, requiring manual start through the **Services** management console or the command-line.

* **Start on installation** Check if you want  to start the Algorand node service and start syncing at end of installation. 

Once you are sure, press Continue and wait the installer to finish.

The installer will create:

* A new windows Service (algodsvc) for controlling the Algorand Node. The name of the service is `algodsvc_` followed by  the network the node is connected to. According to this scheme, your service will be named `algodsvc_testnet`, `algodsvc_betanet` or `algodsvc_mainnet`. 
* "Command Line Tools" shortcut to access the binary directory where all the Algorand tools reside.
* A shortcut to the configuration text file.
* A shortcut to watch the node status in realtime.

At this stage you should be able to click on "Node Status Watch" shortcut in your Start Menu to verify that you are synchronizing.  If you unchecked "Start on installation" you will need to start the service manually.

## Manual Configuration

Please click  the "Configuration" shortcut in your Start Menu, under the "Algorand Node" group, to start the proper `config.json` file.  **To make the changes operative, please restart the Windows service**

## Usage

> :memo: In the following examples, `service_name` is `algodsvc_` followed by  the network the node is connected to. According to this scheme, your service will be named `algodsvc_testnet`, `algodsvc_betanet` or `algodsvc_mainnet`. 

Start the Algorand Service by using the "Services" management console. You can launch the "Run..." panel by pressing Windows + R and executing `services.msc`. Alternatively, you can use the `sc start <service_name>` command in a shell with administrative privileges.

The status of the service can be inspected with the Services management console, or by executing `sc query <service_name>` command in a shell with administrative privileges.

In the same way, stopping the node can be done  with the Services management console, or by executing `sc stop <service_name>` command in a shell with administrative privileges.

> :warning: Forcefully terminating the controlled ALGOD.EXE executable, either by user action or by fatal system error, will trigger the stopping of the Windows service. This will get reported to the Windows Event Log as a 3002 event.

### Monitoring the node

The node can be monitored using the "Node Status Watch" shortcut. Access it by going to your Start Menu, "Algorand ..." group, Node Watch shortcut. This is equivalent of running the `goal node status` command. 

### Using the Windows Event Log

Open the Event Log by pressing Windows + R and executing `eventvwr`. The service will write entries with origin named `Algorand Windows Service`. Current emitted events are on the following list:

| Event ID | Cause |
|----------| ----- |
| 3000 | Service started. |
| 3001 | Algod.exe executable finished normally. |
3002 | Algod.exe executable terminated abnormally.
3003 | Configuration error.
3004 | Algod.exe child process could not be spawned.
3005 | Service stopped.
3006 | Invalid number of arguments for service start.
3007 | Invalid network parameter.
3008 | Invalid, nonexistent or inaccesible node data directory.
3009 | Pre-flight configuration information.

## Building How-TO

The following information applies to power users and developer that are interested in doing a manual build of the Algorand Service and installer.

### Prerequisites

To build the algorand node, service and installer you need  to install the following software distributions:

* **MSYS2 x64** from https://www.msys2.org/.  MSYS2 is an environment to build and run Linux and UNIX software natively in Windows.
* **WiX Toolset** from https://wixtoolset.org (>= v3.11) to compile and build the MSI installer package. 

The two projects contained herein are: 

* **algodsvc** The native Windows Service.
* **installer** The native MSI-based installer.

### Building the go-algorand project

* The installer requires the generated files off the Algorand node code base, referenced in the `go-algorand` submodule. Keep this submodule up-to-date first with:

    ```
    git submodule init
    git submodule update
    ```

    For additional information about working with git submodules see https://git-scm.com/book/en/v2/Git-Tools-Submodules

    The installer mechanism uses the Algorand node versioning scheme for upgrades, so you need to select the release channel you want checking out the proper branch, e.g:

    ```
    cd go-algorand
    git checkout rel/stable
    ```

    Replace `rel/stable` with `rel/nightly` or `rel/beta` if you want to make your installer based on 'bleeding-edge' or beta releases.

* Start your MSYS2-MinGW x64 environment and follow the instructions in steps 1, 2 and 3 at https://developer.algorand.org/tutorials/compile-and-run-the-algorand-node-natively-windows to build the Algorand binaries, but **instead of cloning the go-algorand project, use the `go-algorand` subdirectory in this repository instead**.  

* After the `make` stage finishes, you can do a quick verification checking your node software version:

    ```
    $ $HOME/go/bin/goal.exe --version
    8590162778
    2.3.97114.master [master] (commit #1f59409f)
    go-algorand is licensed with AGPLv3.0
    source code available at https://github.com/algorand/go-algorand
    ```

### Building the Windows Service

Switch to the `algodsvc` subdirectory and execute 

```
make
```

to generate the `algodsvc.exe` binary file.

### Building the Windows MSI Installer

* Switch to the `installer\src` subdirectory.
* Execute `make` with the `NETWORK` parameter specifying which kind of node you want to build (`testnet`, `mainnet` or `betanet` ), e.g to create a testnet node installer you must issue:

    ```
    make NETWORK=testnet
    ```

    The build process should take a while.  It will generate a MSI installer named  `AlgoRand-testnet.msi`  or similar depending on your chosen Algorand network.



## License

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](LICENSE)




