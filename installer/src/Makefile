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
PCHSOURCE=./inc/json.hpp
PCHOUT=json.hpp.gch
CADLL=$(CABASENAME).dll
CC=g++
CFLAGS=-Wall -Wextra -static -shared -pedantic -O2 -g -fanalyzer -D_FORTIFY_SOURCE=2 -fstack-protector -lmsi -municode
SOURCES=AlgorandNode.wxs UI.wxs WelcomeDlg_2.wxs NodeConfigDlg.wxs MessageDlg.wxs
SUFFIX=$(ALGOVERSION)-$(NETWORK)
OBJDIR=./obj-$(SUFFIX)/
MSIDIR=./msi-$(SUFFIX)/
OBJ=$(OBJDIR)AlgorandNode.wixobj 
CANDLE=$(WIX)bin/candle.exe
LIGHT=$(WIX)bin/light.exe
GOBINFOLDER=$(HOME)/go/bin
SVCBINFOLDER=../../algodsvc
BATCHFOLDER=./batch
ALGOVERSION := $(shell $(GOBINFOLDER)/goal.exe --version | awk 'FNR == 2 { print $$1 }' | awk -F "." 'BEGIN{OFS=""} { print $$1,".",$$2,".",$$3 }')
MSI=$(MSIDIR)AlgorandNode-$(SUFFIX)_amd64.msi
THISPRODUCTID := $(shell echo -n $(SUFFIX) | sha256sum | cut -c 1-32 | sed 's/./&-/8;s/./&-/13;s/./&-/18;s/./&-/23')

all: $(PCHOUT) $(CADLL) $(OBJ) $(MSI) 

$(PCHOUT): $(PCHSOURCE)
	$(CC) $(PCHSOURCE) -o $(PCHOUT) $(CFLAGS)

$(CADLL): $(CASOURCE) $(PCHOUT)
	$(CC) $(CASOURCE) -include $(PCHSOURCE) -o $(CADLL) $(CFLAGS)

$(OBJ): $(SOURCES) $(CADLL)
	"$(CANDLE)" $(SOURCES) -ext WixUtilExtension -out "$(OBJDIR)" -arch x64 -dTHISPRODUCTID=$(THISPRODUCTID) -dALGOVERSION=$(ALGOVERSION) -dGOBINFOLDER=$(GOBINFOLDER) -dSVCBINFOLDER=$(SVCBINFOLDER) -dBATCHFOLDER=$(BATCHFOLDER) -dNETWORK=$(NETWORK) -v

$(MSI): $(OBJ)
	"$(LIGHT)" $(OBJDIR)*.wixobj  -ext WixUtilExtension -ext WixUIExtension -out $(MSI) -reusecab -cc cabs -v

clean:
	@rm -f $(CADLL)
	@rm -f *.gch
	@rm -f $(OBJDIR)\*.wixobj
	@rm -f $(MSIDIR)\*.wixpdb
	@rm -f $(MSIDIR)\*.msi





