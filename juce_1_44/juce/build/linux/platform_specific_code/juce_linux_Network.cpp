/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "linuxincludes.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/text/juce_StringArray.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/containers/juce_MemoryBlock.h"

// we'll borrow the mac's socket-based http streaming code..
#include "../../macosx/platform_specific_code/juce_mac_HTTPStream.h"


//==============================================================================
int SystemStats::getMACAddresses (int64* addresses, int maxNum) throw()
{
    // xxx todo
    return 0;
}

END_JUCE_NAMESPACE
