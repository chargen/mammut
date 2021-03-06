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

#ifndef __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
#define __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__

#include "juce_FileFilter.h"
#include "../../../events/juce_ChangeBroadcaster.h"
#include "../../../../juce_core/threads/juce_TimeSliceThread.h"
#include "../../graphics/imaging/juce_Image.h"


//==============================================================================
/**
    A class to asynchronously scan for details about the files in a directory.

    This keeps a list of files and some information about them, using a background
    thread to scan for more files. As files are found, it broadcasts change messages
    to tell any listeners.

    @see FileListComponent, FileBrowserComponent
*/
class JUCE_API  DirectoryContentsList   : public ChangeBroadcaster,
                                          public TimeSliceClient
{
public:
    //==============================================================================
    /** Creates a directory list.

        To set the directory it should point to, use setDirectory(), which will
        also start it scanning for files on the background thread.

        When the background thread finds and adds new files to this list, the
        ChangeBroadcaster class will send a change message, so you can register
        listeners and update them when the list changes.

        @param fileFilter       an optional filter to select which files are
                                included in the list. If this is 0, then all files
                                and directories are included. Make sure that the
                                filter doesn't get deleted during the lifetime of this
                                object
        @param threadToUse      a thread object that this list can use
                                to scan for files as a background task. Make sure
                                that the thread you give it has been started, or you
                                won't get any files!
    */
    DirectoryContentsList (const FileFilter* const fileFilter,
                           TimeSliceThread& threadToUse);

    /** Destructor. */
    ~DirectoryContentsList();


    //==============================================================================
    /** Sets the directory to look in for files.

        If the directory that's passed in is different to the current one, this will
        also start the background thread scanning it for files.
    */
    void setDirectory (const File& directory,
                       const bool includeDirectories,
                       const bool includeFiles);

    /** Returns the directory that's currently being used. */
    const File& getDirectory() const throw();

    /** Clears the list, and stops the thread scanning for files. */
    void clear();

    /** Clears the list and restarts scanning the directory for files. */
    void refresh();

    /** True if the background thread hasn't yet finished scanning for files. */
    bool isStillLoading() const;

    /** Tells the list whether or not to ignore hidden files.

        By default these are ignored.
    */
    void setIgnoresHiddenFiles (const bool shouldIgnoreHiddenFiles);

    //==============================================================================
    /** Contains cached information about one of the files in a DirectoryContentsList.
    */
    struct FileInfo
    {
        //==============================================================================
        /** The filename.

            This isn't a full pathname, it's just the last part of the path, same as you'd
            get from File::getFileName().

            To get the full pathname, use DirectoryContentsList::getDirectory().getChildFile (filename).
        */
        String filename;

        /** File size in bytes. */
        int64 fileSize;

        /** File modification time.

            As supplied by File::getLastModificationTime().
        */
        Time modificationTime;

        /** File creation time.

            As supplied by File::getCreationTime().
        */
        Time creationTime;

        /** True if the file is a directory. */
        bool isDirectory;

        /** True if the file is read-only. */
        bool isReadOnly;
    };

    //==============================================================================
    /** Returns the number of files currently available in the list.

        The info about one of these files can be retrieved with getFileInfo() or
        getFile().

        Obviously as the background thread runs and scans the directory for files, this
        number will change.

        @see getFileInfo, getFile
    */
    int getNumFiles() const;

    /** Returns the cached information about one of the files in the list.

        If the index is in-range, this will return true and will copy the file's details
        to the structure that is passed-in.

        If it returns false, then the index wasn't in range, and the structure won't
        be affected.

        @see getNumFiles, getFile
    */
    bool getFileInfo (const int index,
                      FileInfo& resultInfo) const;

    /** Returns one of the files in the list.

        @param index    should be less than getNumFiles(). If this is out-of-range, the
                        return value will be File::nonexistent
        @see getNumFiles, getFileInfo
    */
    const File getFile (const int index) const;

    /** Returns the file filter being used.

        The filter is specified in the constructor.
    */
    const FileFilter* getFilter() const throw()             { return fileFilter; }

    //==============================================================================
    /** @internal */
    bool useTimeSlice();
    /** @internal */
    TimeSliceThread& getTimeSliceThread() throw()           { return thread; }
    /** @internal */
    static int compareElements (const DirectoryContentsList::FileInfo* const first,
                                const DirectoryContentsList::FileInfo* const second) throw();

    juce_UseDebuggingNewOperator

private:
    File root;
    const FileFilter* fileFilter;
    TimeSliceThread& thread;
    bool includeDirectories, includeFiles, ignoreHiddenFiles;

    CriticalSection fileListLock;
    OwnedArray <FileInfo> files;

    void* volatile fileFindHandle;
    bool volatile shouldStop;

    void changed();
    bool checkNextFile (bool& hasChanged);
    bool addFile (const String& filename, const bool isDir, const bool isHidden,
                  const int64 fileSize, const Time& modTime,
                  const Time& creationTime, const bool isReadOnly);

    DirectoryContentsList (const DirectoryContentsList&);
    const DirectoryContentsList& operator= (const DirectoryContentsList&);
};


#endif   // __JUCE_DIRECTORYCONTENTSLIST_JUCEHEADER__
