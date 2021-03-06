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

#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__
#define __JUCE_CRITICALSECTION_JUCEHEADER__


//==============================================================================
/**
    Prevents multiple threads from accessing shared objects at the same time.

    @see ScopedLock, Thread, InterProcessLock
*/
class JUCE_API  CriticalSection
{
public:
    //==============================================================================
    /**
        Creates a CriticalSection object
    */
    CriticalSection() throw();

    /** Destroys a CriticalSection object.

        If the critical section is deleted whilst locked, its subsequent behaviour
        is unpredictable.
    */
    ~CriticalSection() throw();

    //==============================================================================
    /** Locks this critical section.

        If the lock is currently held by another thread, this will wait until it
        becomes free.

        If the lock is already held by the caller thread, the method returns immediately.

        @see exit, ScopedLock
    */
    void enter() const throw();

    /** Attempts to lock this critical section without blocking.

        This method behaves identically to CriticalSection::enter, except that the caller thread
        does not wait if the lock is currently held by another thread but returns false immediately.

        @returns false if the lock is currently held by another thread, true otherwise.
        @see enter
    */
    bool tryEnter() const throw();

    /** Releases the lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enter() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enter, ScopedLock
    */
    void exit() const throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
#if JUCE_WIN32
  #if JUCE_64BIT
    // To avoid including windows.h in the public Juce includes, we'll just allocate a
    // block of memory here that's big enough to be used internally as a windows critical
    // section object.
    uint8 internal [44];
  #else
    uint8 internal [24];
  #endif
#else
    mutable pthread_mutex_t internal;
#endif

    CriticalSection (const CriticalSection&);
    const CriticalSection& operator= (const CriticalSection&);
};


//==============================================================================
/**
    A class that can be used in place of a real CriticalSection object.

    This is currently used by some templated array classes, and should get
    optimised out by the compiler.

    @see Array, OwnedArray, ReferenceCountedArray
*/
class JUCE_API  DummyCriticalSection
{
public:
    forcedinline DummyCriticalSection() throw()         {}
    forcedinline ~DummyCriticalSection() throw()        {}

    forcedinline void enter() const throw()             {}
    forcedinline void exit() const throw()              {}
};


#endif   // __JUCE_CRITICALSECTION_JUCEHEADER__
