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
