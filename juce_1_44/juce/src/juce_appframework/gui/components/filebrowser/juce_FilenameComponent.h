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

#ifndef __JUCE_FILENAMECOMPONENT_JUCEHEADER__
#define __JUCE_FILENAMECOMPONENT_JUCEHEADER__

#include "../controls/juce_ComboBox.h"
#include "../../../events/juce_AsyncUpdater.h"
#include "../buttons/juce_TextButton.h"
#include "../../../../juce_core/io/files/juce_File.h"
class FilenameComponent;


//==============================================================================
/**
    Listens for events happening to a FilenameComponent.

    Use FilenameComponent::addListener() and FilenameComponent::removeListener() to
    register one of these objects for event callbacks when the filename is changed.

    @See FilenameComponent
*/
class JUCE_API  FilenameComponentListener
{
public:
    /** Destructor. */
    virtual ~FilenameComponentListener() {}

    /** This method is called after the FilenameComponent's file has been changed. */
    virtual void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged) = 0;
};


//==============================================================================
/**
    Shows a filename as an editable text box, with a 'browse' button and a
    drop-down list for recently selected files.

    A handy component for dialogue boxes where you want the user to be able to
    select a file or directory.

    Attach an FilenameComponentListener using the addListener() method, and it will
    get called each time the user changes the filename, either by browsing for a file
    and clicking 'ok', or by typing a new filename into the box and pressing return.

    @see FileChooser, ComboBox
*/
class JUCE_API  FilenameComponent  : public Component,
                                     public SettableTooltipClient,
                                     private AsyncUpdater,
                                     private ButtonListener,
                                     private ComboBoxListener
{
public:
    //==============================================================================
    /** Creates a FilenameComponent.

        @param name             the name for this component.
        @param currentFile      the file to initially show in the box
        @param canEditFilename  if true, the user can manually edit the filename; if false,
                                they can only change it by browsing for a new file
        @param isDirectory      if true, the file will be treated as a directory, and
                                an appropriate directory browser used
        @param isForSaving      if true, the file browser will allow non-existent files to
                                be picked, as the file is assumed to be used for saving rather
                                than loading
        @param fileBrowserWildcard  a wildcard pattern to use in the file browser - e.g. "*.txt;*.foo".
                                If an empty string is passed in, then the pattern is assumed to be "*"
        @param enforcedSuffix   if this is non-empty, it is treated as a suffix that will be added
                                to any filenames that are entered or chosen
        @param textWhenNothingSelected  the message to display in the box before any filename is entered. (This
                                will only appear if the initial file isn't valid)
    */
    FilenameComponent (const String& name,
                       const File& currentFile,
                       const bool canEditFilename,
                       const bool isDirectory,
                       const bool isForSaving,
                       const String& fileBrowserWildcard,
                       const String& enforcedSuffix,
                       const String& textWhenNothingSelected);

    /** Destructor. */
    ~FilenameComponent();

    //==============================================================================
    /** Returns the currently displayed filename. */
    const File getCurrentFile() const;

    /** Changes the current filename.

        If addToRecentlyUsedList is true, the filename will also be added to the
        drop-down list of recent files.

        If sendChangeNotification is false, then the listeners won't be told of the
        change.
    */
    void setCurrentFile (File newFile,
                         const bool addToRecentlyUsedList,
                         const bool sendChangeNotification = true);

    /** Changes whether the use can type into the filename box.
    */
    void setFilenameIsEditable (const bool shouldBeEditable);

    /** Sets a file or directory to be the default starting point for the browser to show.

        This is only used if the current file hasn't been set.
    */
    void setDefaultBrowseTarget (const File& newDefaultDirectory) throw();

    /** Returns all the entries on the recent files list.

        This can be used in conjunction with setRecentlyUsedFilenames() for saving the
        state of this list.

        @see setRecentlyUsedFilenames
    */
    const StringArray getRecentlyUsedFilenames() const;

    /** Sets all the entries on the recent files list.

        This can be used in conjunction with getRecentlyUsedFilenames() for saving the
        state of this list.

        @see getRecentlyUsedFilenames, addRecentlyUsedFile
    */
    void setRecentlyUsedFilenames (const StringArray& filenames);

    /** Adds an entry to the recently-used files dropdown list.

        If the file is already in the list, it will be moved to the top. A limit
        is also placed on the number of items that are kept in the list.

        @see getRecentlyUsedFilenames, setRecentlyUsedFilenames, setMaxNumberOfRecentFiles
    */
    void addRecentlyUsedFile (const File& file);

    /** Changes the limit for the number of files that will be stored in the recent-file list.
    */
    void setMaxNumberOfRecentFiles (const int newMaximum);

    /** Changes the text shown on the 'browse' button.

        By default this button just says "..." but you can change it. The button itself
        can be changed using the look-and-feel classes, so it might not actually have any
        text on it.
    */
    void setBrowseButtonText (const String& browseButtonText);

    //==============================================================================
    /** Adds a listener that will be called when the selected file is changed. */
    void addListener (FilenameComponentListener* const listener) throw();

    /** Removes a previously-registered listener. */
    void removeListener (FilenameComponentListener* const listener) throw();


    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void lookAndFeelChanged();
    /** @internal */
    bool filesDropped (const StringArray& filenames, int mouseX, int mouseY);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    ComboBox* filenameBox;
    String lastFilename;
    Button* browseButton;
    int maxRecentFiles;
    bool isDir, isSaving;
    String wildcard, enforcedSuffix, browseButtonText;
    SortedSet <void*> listeners;
    File defaultBrowseFile;

    void comboBoxChanged (ComboBox*);
    void buttonClicked (Button* button);
    void handleAsyncUpdate();

    FilenameComponent (const FilenameComponent&);
    const FilenameComponent& operator= (const FilenameComponent&);
};



#endif   // __JUCE_FILENAMECOMPONENT_JUCEHEADER__
