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

#ifndef __JUCE_DESKTOP_JUCEHEADER__
#define __JUCE_DESKTOP_JUCEHEADER__

#include "juce_Component.h"
#include "../../application/juce_DeletedAtShutdown.h"
#include "../../events/juce_Timer.h"
#include "../../events/juce_AsyncUpdater.h"
#include "../../../juce_core/containers/juce_SortedSet.h"


//==============================================================================
/**
    Classes can implement this interface and register themselves with the Desktop class
    to receive callbacks when the currently focused component changes.

    @see Desktop::addFocusChangeListener, Desktop::removeFocusChangeListener
*/
class JUCE_API  FocusChangeListener
{
public:
    /** Destructor. */
    virtual ~FocusChangeListener()  {}

    /** Callback to indicate that the currently focused component has changed. */
    virtual void globalFocusChanged (Component* focusedComponent) = 0;
};


//==============================================================================
/**
    Describes and controls aspects of the computer's desktop.

*/
class JUCE_API  Desktop  : private DeletedAtShutdown,
                           private Timer,
                           private AsyncUpdater
{
public:
    //==============================================================================
    /** There's only one dektop object, and this method will return it.
    */
    static Desktop& JUCE_CALLTYPE getInstance() throw();

    //==============================================================================
    /** Returns a list of the positions of all the monitors available.

        The first rectangle in the list will be the main monitor area.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const RectangleList getAllMonitorDisplayAreas (const bool clippedToWorkArea = true) const throw();

    /** Returns the position and size of the main monitor.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const Rectangle getMainMonitorArea (const bool clippedToWorkArea = true) const throw();

    /** Returns the position and size of the monitor which contains this co-ordinate.

        If none of the monitors contains the point, this will just return the
        main monitor.

        If clippedToWorkArea is true, it will exclude any areas like the taskbar on Windows,
        or the menu bar on Mac. If clippedToWorkArea is false, the entire monitor area is returned.
    */
    const Rectangle getMonitorAreaContaining (int x, int y, const bool clippedToWorkArea = true) const throw();


    //==============================================================================
    /** Returns the mouse position.

        The co-ordinates are relative to the top-left of the main monitor.
    */
    static void getMousePosition (int& x, int& y) throw();

    /** Makes the mouse pointer jump to a given location.

        The co-ordinates are relative to the top-left of the main monitor.
    */
    static void setMousePosition (int x, int y) throw();

    /** Returns the last position at which a mouse button was pressed.
    */
    static void getLastMouseDownPosition (int& x, int& y) throw();

    /** Returns the number of times the mouse button has been clicked since the
        app started.

        Each mouse-down event increments this number by 1.
    */
    static int getMouseButtonClickCounter() throw();

    //==============================================================================
    /** This lets you prevent the screensaver from becoming active.

        Handy if you're running some sort of presentation app where having a screensaver
        appear would be annoying.

        Pass false to disable the screensaver, and true to re-enable it. (Note that this
        won't enable a screensaver unless the user has actually set one up).

        The disablement will only happen while the Juce application is the foreground
        process - if another task is running in front of it, then the screensaver will
        be unaffected.

        @see isScreenSaverEnabled
    */
    static void setScreenSaverEnabled (const bool isEnabled) throw();

    /** Returns true if the screensaver has not been turned off.

        This will return the last value passed into setScreenSaverEnabled(). Note that
        it won't tell you whether the user is actually using a screen saver, just
        whether this app is deliberately preventing one from running.

        @see setScreenSaverEnabled
    */
    static bool isScreenSaverEnabled() throw();

    //==============================================================================
    /** Registers a MouseListener that will receive all mouse events that occur on
        any component.

        @see removeGlobalMouseListener
    */
    void addGlobalMouseListener (MouseListener* const listener) throw();

    /** Unregisters a MouseListener that was added with the addGlobalMouseListener()
        method.

        @see addGlobalMouseListener
    */
    void removeGlobalMouseListener (MouseListener* const listener) throw();

    //==============================================================================
    /** Registers a MouseListener that will receive a callback whenever the focused
        component changes.
    */
    void addFocusChangeListener (FocusChangeListener* const listener) throw();

    /** Unregisters a listener that was added with addFocusChangeListener(). */
    void removeFocusChangeListener (FocusChangeListener* const listener) throw();

    //==============================================================================
    /** Returns the number of components that are currently active as top-level
        desktop windows.

        @see getComponent, Component::addToDesktop
    */
    int getNumComponents() const throw();

    /** Returns one of the top-level desktop window components.

        The index is from 0 to getNumComponents() - 1. This could return 0 if the
        index is out-of-range.

        @see getNumComponents, Component::addToDesktop
    */
    Component* getComponent (const int index) const throw();

    /** Finds the component at a given screen location.

        This will drill down into top-level windows to find the child component at
        the given position.

        Returns 0 if the co-ordinates are inside a non-Juce window.
    */
    Component* findComponentAt (const int screenX,
                                const int screenY) const;


    //==============================================================================
    juce_UseDebuggingNewOperator

    /** Tells this object to refresh its idea of what the screen resolution is.

        (Called internally by the native code).
    */
    void refreshMonitorSizes() throw();

    /** True if the OS supports semitransparent windows */
    static bool canUseSemiTransparentWindows() throw();


private:
    //==============================================================================
    friend class Component;
    friend class ComponentPeer;
    SortedSet <void*> mouseListeners, focusListeners;
    VoidArray desktopComponents;

    friend class DeletedAtShutdown;
    friend class TopLevelWindowManager;
    Desktop() throw();
    ~Desktop() throw();

    Array <Rectangle> monitorCoordsClipped, monitorCoordsUnclipped;
    int lastMouseX, lastMouseY;

    void timerCallback();
    void sendMouseMove();
    void resetTimer() throw();

    int getNumDisplayMonitors() const throw();
    const Rectangle getDisplayMonitorCoordinates (const int index, const bool clippedToWorkArea) const throw();

    void addDesktopComponent (Component* const c) throw();
    void removeDesktopComponent (Component* const c) throw();
    void componentBroughtToFront (Component* const c) throw();

    void triggerFocusCallback() throw();
    void handleAsyncUpdate();

    Desktop (const Desktop&);
    const Desktop& operator= (const Desktop&);
};


#endif   // __JUCE_DESKTOP_JUCEHEADER__
