Algorand Node Windows Installer
===============================

ChangeLog
---------

**V1.0.0 (210119)**: Initial version.


How to build
------------

* Install Wix  Toolset (https://wixtoolset.org/).  Version 3.11 or higher is required.
* Build the algorand node following instructions at https://developer.algorand.org/tutorials/compile-and-run-the-algorand-node-natively-windows/ until step 3. Make sure `make` ends successfully.
* Build the Windows Service at this repository under directory `windows/algodsvc`. Follow the `README.md` steps there.
* Execute `make` with the `NETWORK` parameter specifying which kind of node you want to build, e.g to create a testnet node installer you must issue:

    ```
    make NETWORK=testnet
    ```

    The build process should take a while.  It will generate a MSI installer named  `algorand-node-testnet.msi`  or similar depending on your chosen Algorand network.

Installation
------------

> :warning: 32-bit Windows is not supported by the installer.

To launch the installation process just double click the MSI file, accept the license agreement and wait for the installer to finish.  
The installer will create:

* A new windows Service (algodsvc) for controlling the Algorand Node.
* "Command Line Tools" shortcut to access the binary directory where all the Algorand tools reside.
* A shortcut to the configuration text file.
* A shortcut to watch the node status in realtime.

Configuration
-------------

Please click  the "Configuration" shortcut in your Start Menu, under the "Algorand Node" group, to start the proper `config.json` file. You need at least to modify two entries for a successful bootup of your node:

* `DNSBootstrapID`:  set to `betanet.algorand.network`, `mainnet.algorand.network` or `testnet.algorand.network` depending on your chosen node network type.
* `EndpointAddress `: set to local host and port you would like to use (`127.0.0.1:8080` for example).  Keep in mind that you may need to setup your firewall depending on the port number.

Save the configuration file and start the Algorand Service by using the "Services" control panel. You can launch the "Run..." panel by pressing Windows + R and executing `services.msc`. Alternatively, you can use the `sc start algodsvc` command in a shell with administrative privileges.

Monitoring
----------

The service and controlled-node operation can be monitored using the Node Watch tool and/or the Event Viewer.

License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
