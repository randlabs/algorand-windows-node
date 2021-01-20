# Algorand Node for Windows

This repository contains the supporting software for running an Algorand Node under Windows systems, including a native Windows service, companion tools and proper MSI based installer and uninstaller.

> :warning: 32-bit Windows is not supported.

## Prerequisites

To build the algorand node, service and installer you need  to install the following software distributions:

* **MSYS2 x64** from https://www.msys2.org/.  MSYS2 is an environment to build and run Linux and UNIX software natively in Windows.
* **WiX Toolset** from https://wixtoolset.org (>= v3.11) to compile and build the MSI installer package. 

## Building How-TO

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


## Installation

To launch the installation process just double click the MSI file, accept the license agreement and wait for the installer to finish.  
The installer will create:

* A new windows Service (algodsvc) for controlling the Algorand Node.
* "Command Line Tools" shortcut to access the binary directory where all the Algorand tools reside.
* A shortcut to the configuration text file.
* A shortcut to watch the node status in realtime.

## Configuration

Please click  the "Configuration" shortcut in your Start Menu, under the "Algorand Node" group, to start the proper `config.json` file. You need at least to modify two entries for a successful bootup of your node:

* `DNSBootstrapID`:  set to `betanet.algorand.network`, `mainnet.algorand.network` or `testnet.algorand.network` depending on your chosen node network type.
* `EndpointAddress `: set to local host and port you would like to use (`127.0.0.1:8080` for example).  Keep in mind that you may need to setup your firewall depending on the port number.

Save the configuration file. 

## Usage

### Starting and stopping the node

Start the Algorand Service by using the "Services" management console. You can launch the "Run..." panel by pressing Windows + R and executing `services.msc`. Alternatively, you can use the `sc start algodsvc` command in a shell with administrative privileges.

The status of the service can be inspected with the Services management console, or by executing `sc query algodsvc` command in a shell with administrative privileges.

In the same way, stopping the node can be done  with the Services management console, or by executing `sc stop algodsvc` command in a shell with administrative privileges.

> :warning: Forcefully terminating the controlled ALGOD.EXE executable, either by user action or by fatal system error, will trigger the stopping of the Windows service. This will get reported to the Windows Event Log as a 3002 event.

### Monitoring the node

The node can be monitored using the "Node Watch tool". Access it by going to your Start Menu, "Algorand ..." group, Node Watch shortcut. This is equivalent of running the `goal node status` command. 

### Using the Windows Event Log

Open the Event Log by pressing Windows + R and executing `eventvwr`. The service will write entries with origin named `Algorand Windows Service`. It will write a set of log entries caused by different situations, as follows:

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

## License

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](LICENSE)




