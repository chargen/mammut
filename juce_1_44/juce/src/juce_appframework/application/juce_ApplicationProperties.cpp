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

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationProperties.h"
#include "../gui/components/windows/juce_AlertWindow.h"
#include "../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
juce_ImplementSingleton (ApplicationProperties)


//==============================================================================
ApplicationProperties::ApplicationProperties() throw()
    : userProps (0),
      commonProps (0),
      msBeforeSaving (3000),
      options (PropertiesFile::storeAsBinary),
      commonSettingsAreReadOnly (0)
{
}

ApplicationProperties::~ApplicationProperties()
{
    closeFiles();
    clearSingletonInstance();
}

//==============================================================================
void ApplicationProperties::setStorageParameters (const String& applicationName,
                                                  const String& fileNameSuffix,
                                                  const String& folderName_,
                                                  const int millisecondsBeforeSaving,
                                                  const int propertiesFileOptions) throw()
{
    appName = applicationName;
    fileSuffix = fileNameSuffix;
    folderName = folderName_;
    msBeforeSaving = millisecondsBeforeSaving;
    options = propertiesFileOptions;
}

bool ApplicationProperties::testWriteAccess (const bool testUserSettings,
                                             const bool testCommonSettings,
                                             const bool showWarningDialogOnFailure)
{
    const bool userOk    = (! testUserSettings)   || getUserSettings()->save();
    const bool commonOk  = (! testCommonSettings) || getCommonSettings (false)->save();

    if (! (userOk && commonOk))
    {
        if (showWarningDialogOnFailure)
        {
            String filenames;

            if (userProps != 0 && ! userOk)
                filenames << '\n' << userProps->getFile().getFullPathName();

            if (commonProps != 0 && ! commonOk)
                filenames << '\n' << commonProps->getFile().getFullPathName();

            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         appName + TRANS(" - Unable to save settings"),
                                         TRANS("An error occurred when trying to save the application's settings file...\n\nIn order to save and restore its settings, ")
                                          + appName + TRANS(" needs to be able to write to the following files:\n")
                                          + filenames
                                          + TRANS("\n\nMake sure that these files aren't read-only, and that the disk isn't full."));
        }

        return false;
    }

    return true;
}

//==============================================================================
void ApplicationProperties::openFiles() throw()
{
    // You need to call setStorageParameters() before trying to get hold of the
    // properties!
    jassert (appName.isNotEmpty());

    if (appName.isNotEmpty())
    {
        if (userProps == 0)
            userProps = PropertiesFile::createDefaultAppPropertiesFile (appName, fileSuffix, folderName,
                                                                        false, msBeforeSaving, options);

        if (commonProps == 0)
            commonProps = PropertiesFile::createDefaultAppPropertiesFile (appName, fileSuffix, folderName,
                                                                          true, msBeforeSaving, options);

        userProps->setFallbackPropertySet (commonProps);
    }
}

PropertiesFile* ApplicationProperties::getUserSettings() throw()
{
    if (userProps == 0)
        openFiles();

    return userProps;
}

PropertiesFile* ApplicationProperties::getCommonSettings (const bool returnUserPropsIfReadOnly) throw()
{
    if (commonProps == 0)
        openFiles();

    if (returnUserPropsIfReadOnly)
    {
        if (commonSettingsAreReadOnly == 0)
            commonSettingsAreReadOnly = commonProps->save() ? -1 : 1;

        if (commonSettingsAreReadOnly > 0)
            return userProps;
    }

    return commonProps;
}

bool ApplicationProperties::saveIfNeeded()
{
    return (userProps == 0 || userProps->saveIfNeeded())
         && (commonProps == 0 || commonProps->saveIfNeeded());
}

void ApplicationProperties::closeFiles()
{
    deleteAndZero (userProps);
    deleteAndZero (commonProps);
}


END_JUCE_NAMESPACE
