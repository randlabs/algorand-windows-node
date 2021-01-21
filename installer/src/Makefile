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
SOURCES=AlgorandNode.wxs UI.wxs WelcomeDlg_2.wxs NodeConfigDlg.wxs
OBJ=AlgorandNode.wixobj WelcomeDlg_2.wixobj UI.wixobj NodeConfigDlg.wixobj
CANDLE=$(WIX)bin\candle.exe
LIGHT=$(WIX)bin\light.exe
GOBINFOLDER=$(HOME)/go/bin
SVCBINFOLDER=../../algodsvc
ALGOVERSION := $(shell $(GOBINFOLDER)/goal.exe --version | awk 'FNR == 2 { print $$1 }' | awk -F "." 'BEGIN{OFS=""} { print $$1,".",$$2,".",$$3 }')
MSI=AlgorandNode-$(NETWORK)-$(ALGOVERSION).msi

all: $(OBJ) $(MSI)

$(OBJ): $(SOURCES)
	"$(CANDLE)" $(SOURCES) -arch x64 -dALGOVERSION=$(ALGOVERSION) -dGOBINFOLDER=$(GOBINFOLDER) -dSVCBINFOLDER=$(SVCBINFOLDER) -dNETWORK=$(NETWORK) -v

$(MSI): $(OBJ)
	"$(LIGHT)" $(OBJ) -ext WixUIExtension -out $(MSI) -reusecab -cc cabs -v

clean:
	@rm -f *.wixobj
	@rm -f *.wixpdb
	@rm -f *.msi





