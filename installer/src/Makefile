#
# Makefile for WiX-based windows Installer
# Author: Hernán Di Pietro <hernan@randlabs.io>
# (c) 2021 Rand Labs.
#
# Licensed under AGPL3.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------
#
# Product GUIDs are correlated to the x.y.z version and network of the product
# by calculating SHA256 of "x.y.z-network" string and formatting it as GUID, e.g:
#
# * See GenProdId.sh for details. For example, 2.3.0-testnet Producit GUID is 
#   14df32b6-c41a-3fde-1a39-79f7b7f27ef6 
# -----------------------------------------------------------------------------

CABASENAME=CustomActions
CASOURCE=$(CABASENAME).cpp
CADLL=$(CABASENAME).dll
CC=g++
CFLAGS=-Wall -Wextra -static -shared -pedantic -O2 -g -fanalyzer -DFORTIFY_SOURCE=2 -fstack-protector -lmsi -municode
SOURCES=AlgorandNode.wxs UI.wxs WelcomeDlg_2.wxs NodeConfigDlg.wxs MessageDlg.wxs
OBJ=AlgorandNode.wixobj WelcomeDlg_2.wixobj UI.wixobj NodeConfigDlg.wixobj MessageDlg.wixobj
CANDLE=$(WIX)bin\candle.exe
LIGHT=$(WIX)bin\light.exe
GOBINFOLDER=$(HOME)/go/bin
SVCBINFOLDER=../../algodsvc
ALGOVERSION := $(shell $(GOBINFOLDER)/goal.exe --version | awk 'FNR == 2 { print $$1 }' | awk -F "." 'BEGIN{OFS=""} { print $$1,".",$$2,".",$$3 }')
MSI=AlgorandNode-$(ALGOVERSION)-$(NETWORK).msi
THISPRODUCTID := $(shell echo -n $(ALGOVERSION)-$(NETWORK) | sha256sum | cut -c 1-32 | sed 's/./&-/8;s/./&-/13;s/./&-/18;s/./&-/23')

all: $(CADLL) $(OBJ) $(MSI) 

$(CADLL): $(CASOURCE)
	$(CC) $(CASOURCE)  -o $(CADLL) $(CFLAGS)

$(OBJ): $(SOURCES) $(CADLL)
	"$(CANDLE)" $(SOURCES) -arch x64 -dTHISPRODUCTID=$(THISPRODUCTID) -dALGOVERSION=$(ALGOVERSION) -dGOBINFOLDER=$(GOBINFOLDER) -dSVCBINFOLDER=$(SVCBINFOLDER) -dNETWORK=$(NETWORK) -v

$(MSI): $(OBJ)
	"$(LIGHT)" $(OBJ) -ext WixUIExtension -out $(MSI) -reusecab -cc cabs -v

clean:
	@rm -f $(CADLL)
	@rm -f *.wixobj
	@rm -f *.wixpdb
	@rm -f *.msi





