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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ListBox.h"
#include "../../graphics/geometry/juce_RectangleList.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../mouse/juce_DragAndDropContainer.h"


//==============================================================================
class ListBoxRowComponent  : public Component
{
public:
    ListBoxRowComponent (ListBox& owner_)
        : owner (owner_),
          row (-1),
          selected (false),
          isDragging (false)
    {
    }

    ~ListBoxRowComponent()
    {
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        if (owner.getModel() != 0)
            owner.getModel()->paintListBoxItem (row, g, getWidth(), getHeight(), selected);
    }

    void update (const int row_, const bool selected_)
    {
        if (row != row_ || selected != selected_)
        {
            repaint();
            row = row_;
            selected = selected_;
        }

        if (owner.getModel() != 0)
        {
            Component* const customComp = owner.getModel()->refreshComponentForRow (row_, selected_, getChildComponent (0));

            if (customComp != 0)
            {
                addAndMakeVisible (customComp);
                customComp->setBounds (0, 0, getWidth(), getHeight());

                for (int i = getNumChildComponents(); --i >= 0;)
                    if (getChildComponent (i) != customComp)
                        delete getChildComponent (i);
            }
            else
            {
                deleteAllChildren();
            }
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        isDragging = false;
        selectRowOnMouseUp = false;

        if (isEnabled())
        {
            if (! selected)
            {
                owner.selectRowsBasedOnModifierKeys (row, e.mods);

                if (owner.getModel() != 0)
                    owner.getModel()->listBoxItemClicked (row, e);
            }
            else
            {
                selectRowOnMouseUp = true;
            }
        }
    }

    void mouseUp (const MouseEvent& e)
    {
        if (isEnabled() && selectRowOnMouseUp && ! isDragging)
        {
            owner.selectRowsBasedOnModifierKeys (row, e.mods);

            if (owner.getModel() != 0)
                owner.getModel()->listBoxItemClicked (row, e);
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        if (owner.getModel() != 0 && isEnabled())
            owner.getModel()->listBoxItemDoubleClicked (row, e);
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (isEnabled() && owner.getModel() != 0 && ! (e.mouseWasClicked() || isDragging))
        {
            const SparseSet <int> selectedRows (owner.getSelectedRows());

            if (selectedRows.size() > 0)
            {
                const String dragDescription (owner.getModel()->getDragSourceDescription (selectedRows));

                if (dragDescription.isNotEmpty())
                {
                    isDragging = true;

                    DragAndDropContainer* const dragContainer
                        = DragAndDropContainer::findParentDragContainerFor (this);

                    if (dragContainer != 0)
                    {
                        Image* dragImage = owner.createSnapshotOfSelectedRows();
                        dragImage->multiplyAllAlphas (0.6f);

                        dragContainer->startDragging (dragDescription, &owner, dragImage);
                    }
                    else
                    {
                        // to be able to do a drag-and-drop operation, the listbox needs to
                        // be inside a component which is also a DragAndDropContainer.
                        jassertfalse
                    }
                }
            }
        }
    }

    void resized()
    {
        if (getNumChildComponents() > 0)
            getChildComponent(0)->setBounds (0, 0, getWidth(), getHeight());
    }

    juce_UseDebuggingNewOperator

    bool neededFlag;

private:
    ListBox& owner;
    int row;
    bool selected, isDragging, selectRowOnMouseUp;

    ListBoxRowComponent (const ListBoxRowComponent&);
    const ListBoxRowComponent& operator= (const ListBoxRowComponent&);
};


//==============================================================================
class ListViewport  : public Viewport
{
public:
    int firstIndex, firstWholeIndex, lastWholeIndex;
    bool hasUpdated;

    //==============================================================================
    ListViewport (ListBox& owner_)
        : owner (owner_)
    {
        setWantsKeyboardFocus (false);

        setViewedComponent (new Component());
        getViewedComponent()->addMouseListener (this, false);
        getViewedComponent()->setWantsKeyboardFocus (false);
    }

    ~ListViewport()
    {
        getViewedComponent()->removeMouseListener (this);
        getViewedComponent()->deleteAllChildren();
    }

    ListBoxRowComponent* getComponentForRow (const int row) const
    {
        return (ListBoxRowComponent*) getViewedComponent()->getChildComponent (row % jmax (1, getViewedComponent()->getNumChildComponents()));
    }

    Component* getComponentForRowIfOnscreen (const int row) const
    {
        return (row >= firstIndex && row < firstIndex + getViewedComponent()->getNumChildComponents())
                 ? getComponentForRow (row) : 0;
    }

    void visibleAreaChanged (int, int, int, int)
    {
        updateVisibleArea (true);

        if (owner.getModel() != 0)
            owner.getModel()->listWasScrolled();
    }

    void updateVisibleArea (const bool makeSureItUpdatesContent)
    {
        hasUpdated = false;

        int newX = getViewedComponent()->getX();
        int newY = getViewedComponent()->getY();
        int newW = jmax (owner.minimumRowWidth, getMaximumVisibleWidth());
        int newH = owner.totalItems * owner.getRowHeight();

        if (newY + newH < getMaximumVisibleHeight() && newH > getMaximumVisibleHeight())
            newY = getMaximumVisibleHeight() - newH;

        getViewedComponent()->setBounds (newX, newY, newW, newH);

        if (makeSureItUpdatesContent && ! hasUpdated)
            updateContents();
    }

    void updateContents()
    {
        hasUpdated = true;
        const int rowHeight = owner.getRowHeight();

        if (rowHeight > 0)
        {
            const int y = getViewPositionY();
            const int w = getViewedComponent()->getWidth();

            const int numNeeded = 2 + getMaximumVisibleHeight() / rowHeight;

            while (numNeeded > getViewedComponent()->getNumChildComponents())
                getViewedComponent()->addAndMakeVisible (new ListBoxRowComponent (owner));

            jassert (numNeeded >= 0);

            while (numNeeded < getViewedComponent()->getNumChildComponents())
            {
                Component* const rowToRemove
                    = getViewedComponent()->getChildComponent (getViewedComponent()->getNumChildComponents() - 1);

                delete rowToRemove;
            }

            firstIndex = y / rowHeight;
            firstWholeIndex = (y + rowHeight - 1) / rowHeight;
            lastWholeIndex = (y + getMaximumVisibleHeight() - 1) / rowHeight;

            for (int i = 0; i < numNeeded; ++i)
            {
                const int row = i + firstIndex;
                ListBoxRowComponent* const rowComp = getComponentForRow (row);

                if (rowComp != 0)
                {
                    rowComp->setBounds (0, row * rowHeight, w, rowHeight);
                    rowComp->update (row, owner.isRowSelected (row));
                }
            }
        }

        if (owner.headerComponent != 0)
            owner.headerComponent->setBounds (owner.outlineThickness + getViewedComponent()->getX(),
                                              owner.outlineThickness,
                                              jmax (owner.getWidth() - owner.outlineThickness * 2,
                                                    getViewedComponent()->getWidth()),
                                              owner.headerComponent->getHeight());
    }

    void paint (Graphics& g)
    {
        if (isOpaque())
            g.fillAll (owner.findColour (ListBox::backgroundColourId));
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ListBox& owner;

    ListViewport (const ListViewport&);
    const ListViewport& operator= (const ListViewport&);
};


//==============================================================================
ListBox::ListBox (const String& name, ListBoxModel* const model_)
    : Component (name),
      model (model_),
      headerComponent (0),
      totalItems (0),
      rowHeight (22),
      minimumRowWidth (0),
      scrollBarSize (18),
      outlineThickness (0),
      lastRowSelected (-1),
      mouseMoveSelects (false),
      multipleSelection (false),
      hasDoneInitialUpdate (false)
{
    addAndMakeVisible (viewport = new ListViewport (*this));

    setWantsKeyboardFocus (true);
}

ListBox::~ListBox()
{
    deleteAllChildren();
}

void ListBox::setModel (ListBoxModel* const newModel)
{
    if (model != newModel)
    {
        model = newModel;
        updateContent();
    }
}

void ListBox::setMultipleSelectionEnabled (bool b)
{
    multipleSelection = b;
}

void ListBox::setMouseMoveSelectsRows (bool b)
{
    mouseMoveSelects = b;

    if (b)
        addMouseListener (this, true);
}

//==============================================================================
void ListBox::paint (Graphics& g)
{
    if (! hasDoneInitialUpdate)
        updateContent();

    g.fillAll (findColour (backgroundColourId));
}

void ListBox::paintOverChildren (Graphics& g)
{
    if (outlineThickness > 0)
    {
        g.setColour (findColour (outlineColourId));
        g.drawRect (0, 0, getWidth(), getHeight(), outlineThickness);
    }
}

void ListBox::resized()
{
    viewport->setBoundsInset (BorderSize (outlineThickness + ((headerComponent != 0) ? headerComponent->getHeight() : 0),
                                          outlineThickness,
                                          outlineThickness,
                                          outlineThickness));

    viewport->setSingleStepSizes (20, getRowHeight());

    viewport->updateVisibleArea (false);
}

void ListBox::visibilityChanged()
{
    viewport->updateVisibleArea (true);
}

//==============================================================================
void ListBox::updateContent()
{
    hasDoneInitialUpdate = true;
    totalItems = (model != 0) ? model->getNumRows() : 0;

    bool selectionChanged = false;

    if (selected [selected.size() - 1] >= totalItems)
    {
        selected.removeRange (totalItems, INT_MAX - totalItems);

        if (selected.size() == 0)
            lastRowSelected = totalItems - 1;
        else if (lastRowSelected >= totalItems)
            lastRowSelected = getSelectedRow (0);

        selectionChanged = true;
    }

    viewport->updateVisibleArea (isVisible());
    viewport->resized();

    if (selectionChanged && model != 0)
        model->selectedRowsChanged (lastRowSelected);
}

//==============================================================================
void ListBox::selectRow (const int row,
                         bool dontScroll,
                         bool deselectOthersFirst)
{
    selectRowInternal (row, dontScroll, deselectOthersFirst, false);
}

void ListBox::selectRowInternal (const int row,
                                 bool dontScroll,
                                 bool deselectOthersFirst,
                                 bool isMouseClick)
{
    if (! multipleSelection)
        deselectOthersFirst = true;

    if ((! isRowSelected (row))
         || (deselectOthersFirst && getNumSelectedRows() > 1))
    {
        if (row >= 0 && row < totalItems)
        {
            if (deselectOthersFirst)
                selected.clear();

            selected.addRange (row, 1);

            if (getHeight() == 0 || getWidth() == 0)
                dontScroll = true;

            viewport->hasUpdated = false;

            if (row < viewport->firstWholeIndex && ! dontScroll)
            {
                viewport->setViewPosition (viewport->getViewPositionX(),
                                           row * getRowHeight());
            }
            else if (row >= viewport->lastWholeIndex && ! dontScroll)
            {
                const int rowsOnScreen = viewport->lastWholeIndex - viewport->firstWholeIndex;

                if (row >= lastRowSelected + rowsOnScreen
                     && rowsOnScreen < totalItems - 1
                     && ! isMouseClick)
                {
                    viewport->setViewPosition (viewport->getViewPositionX(),
                                               jlimit (0, jmax (0, totalItems - rowsOnScreen), row)
                                                   * getRowHeight());
                }
                else
                {
                    viewport->setViewPosition (viewport->getViewPositionX(),
                                               jmax (0, (row  + 1) * getRowHeight() - viewport->getMaximumVisibleHeight()));
                }
            }

            if (! viewport->hasUpdated)
                viewport->updateContents();

            lastRowSelected = row;
            model->selectedRowsChanged (row);
        }
        else
        {
            if (deselectOthersFirst)
                deselectAllRows();
        }
    }
}

void ListBox::deselectRow (const int row)
{
    if (selected.contains (row))
    {
        selected.removeRange (row, 1);

        if (row == lastRowSelected)
            lastRowSelected = getSelectedRow (0);

        viewport->updateContents();
        model->selectedRowsChanged (lastRowSelected);
    }
}

void ListBox::setSelectedRows (const SparseSet<int>& setOfRowsToBeSelected)
{
    selected = setOfRowsToBeSelected;
    selected.removeRange (totalItems, INT_MAX - totalItems);

    if (! isRowSelected (lastRowSelected))
        lastRowSelected = -1;

    viewport->updateContents();

    if (model != 0)
        model->selectedRowsChanged (lastRowSelected);
}

const SparseSet<int> ListBox::getSelectedRows() const
{
    return selected;
}

void ListBox::selectRangeOfRows (int firstRow, int lastRow)
{
    if (multipleSelection && (firstRow != lastRow))
    {
        const int numRows = totalItems - 1;
        firstRow = jlimit (0, jmax (0, numRows), firstRow);
        lastRow = jlimit (0, jmax (0, numRows), lastRow);

        selected.addRange (jmin (firstRow, lastRow),
                           abs (firstRow - lastRow) + 1);

        selected.removeRange (lastRow, 1);
    }

    selectRowInternal (lastRow, false, false, true);
}

void ListBox::flipRowSelection (const int row)
{
    if (isRowSelected (row))
        deselectRow (row);
    else
        selectRowInternal (row, false, false, true);
}

void ListBox::deselectAllRows()
{
    if (! selected.isEmpty())
    {
        selected.clear();
        lastRowSelected = -1;

        viewport->updateContents();

        if (model != 0)
            model->selectedRowsChanged (lastRowSelected);
    }
}

void ListBox::selectRowsBasedOnModifierKeys (const int row,
                                             const ModifierKeys& mods)
{
    if (multipleSelection && mods.isCommandDown())
    {
        flipRowSelection (row);
    }
    else if (multipleSelection && mods.isShiftDown() && lastRowSelected >= 0)
    {
        selectRangeOfRows (lastRowSelected, row);
    }
    else
    {
        selectRowInternal (row, false, true, true);
    }
}

int ListBox::getNumSelectedRows() const
{
    return selected.size();
}

int ListBox::getSelectedRow (const int index) const
{
    return (index >= 0 && index < selected.size())
                ? selected [index] : -1;
}

bool ListBox::isRowSelected (const int row) const
{
    return selected.contains (row);
}

int ListBox::getLastRowSelected() const
{
    return (isRowSelected (lastRowSelected)) ? lastRowSelected : -1;
}

//==============================================================================
int ListBox::getRowContainingPosition (const int x, const int y) const
{
    if (x >= 0 && x < getWidth())
    {
        const int row = (viewport->getViewPositionY() + y - viewport->getY()) / rowHeight;

        if (row >= 0 && row < totalItems)
            return row;
    }

    return -1;
}

Component* ListBox::getComponentForRowNumber (const int row) const
{
    Component* const listRowComp = viewport->getComponentForRowIfOnscreen (row);
    return listRowComp != 0 ? listRowComp->getChildComponent (0) : 0;
}

const Rectangle ListBox::getRowPosition (const int rowNumber) const
{
    const int rowHeight = getRowHeight();

    return Rectangle (0, rowHeight * rowNumber,
                      viewport->getViewedComponent()->getWidth(), rowHeight);
}

void ListBox::setVerticalPosition (const double proportion)
{
    const int offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    viewport->setViewPosition (viewport->getViewPositionX(),
                               jmax (0, roundDoubleToInt (proportion * offscreen)));
}

double ListBox::getVerticalPosition() const
{
    const int offscreen = viewport->getViewedComponent()->getHeight() - viewport->getHeight();

    return (offscreen > 0) ? viewport->getViewPositionY() / (double) offscreen
                           : 0;
}

int ListBox::getVisibleRowWidth() const
{
    return viewport->getViewWidth();
}

void ListBox::scrollToEnsureRowIsOnscreen (const int row)
{
    if (row < viewport->firstWholeIndex)
    {
        viewport->setViewPosition (viewport->getViewPositionX(),
                                   row * getRowHeight());
    }
    else if (row >= viewport->lastWholeIndex)
    {
        viewport->setViewPosition (viewport->getViewPositionX(),
                                   jmax (0, (row  + 1) * getRowHeight() - viewport->getMaximumVisibleHeight()));
    }
}

//==============================================================================
bool ListBox::keyPressed (const KeyPress& key)
{
    const int numVisibleRows = viewport->getHeight() / getRowHeight();

    const bool multiple = multipleSelection
                            && (lastRowSelected >= 0)
                            && (key.getModifiers().isShiftDown()
                                 || key.getModifiers().isCtrlDown()
                                 || key.getModifiers().isCommandDown());

    if (key.isKeyCode (KeyPress::upKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - 1);
        else
            selectRow (jmax (0, lastRowSelected - 1));
    }
    else if (key.isKeyCode (KeyPress::returnKey)
              && isRowSelected (lastRowSelected))
    {
        if (model != 0)
            model->returnKeyPressed (lastRowSelected);
    }
    else if (key.isKeyCode (KeyPress::pageUpKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected - numVisibleRows);
        else
            selectRow (jmax (0, jmax (0, lastRowSelected) - numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::pageDownKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + numVisibleRows);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected) + numVisibleRows));
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        if (multiple && key.getModifiers().isShiftDown())
            selectRangeOfRows (lastRowSelected, 0);
        else
            selectRow (0);
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        if (multiple && key.getModifiers().isShiftDown())
            selectRangeOfRows (lastRowSelected, totalItems - 1);
        else
            selectRow (totalItems - 1);
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        if (multiple)
            selectRangeOfRows (lastRowSelected, lastRowSelected + 1);
        else
            selectRow (jmin (totalItems - 1, jmax (0, lastRowSelected) + 1));
    }
    else if ((key.isKeyCode (KeyPress::deleteKey) || key.isKeyCode (KeyPress::backspaceKey))
               && isRowSelected (lastRowSelected))
    {
        if (model != 0)
            model->deleteKeyPressed (lastRowSelected);
    }
    else if (multiple && key == KeyPress (T('a'), ModifierKeys::commandModifier, 0))
    {
        selectRangeOfRows (0, INT_MAX);
    }
    else
    {
        return false;
    }

    return true;
}

bool ListBox::keyStateChanged()
{
    return KeyPress::isKeyCurrentlyDown (KeyPress::upKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::pageUpKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::downKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::pageDownKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::homeKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::endKey)
        || KeyPress::isKeyCurrentlyDown (KeyPress::returnKey);
}

void ListBox::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    getHorizontalScrollBar()->mouseWheelMove (e, wheelIncrementX, 0);
    getVerticalScrollBar()->mouseWheelMove (e, 0, wheelIncrementY);
}

void ListBox::mouseMove (const MouseEvent& e)
{
    if (mouseMoveSelects)
    {
        const MouseEvent e2 (e.getEventRelativeTo (this));

        selectRow (getRowContainingPosition (e2.x, e2.y), true);

        lastMouseX = e2.x;
        lastMouseY = e2.y;
    }
}

void ListBox::mouseExit (const MouseEvent& e)
{
    mouseMove (e);
}

void ListBox::mouseUp (const MouseEvent& e)
{
    if (e.mouseWasClicked() && model != 0)
        model->backgroundClicked();
}

//==============================================================================
void ListBox::setRowHeight (const int newHeight)
{
    rowHeight = jmax (1, newHeight);
    viewport->setSingleStepSizes (20, rowHeight);
    updateContent();
}

int ListBox::getNumRowsOnScreen() const throw()
{
    return viewport->getMaximumVisibleHeight() / rowHeight;
}

void ListBox::setMinimumContentWidth (const int newMinimumWidth)
{
    minimumRowWidth = newMinimumWidth;
    updateContent();
}

int ListBox::getVisibleContentWidth() const
{
    return viewport->getMaximumVisibleWidth();
}

ScrollBar* ListBox::getVerticalScrollBar() const
{
    return viewport->getVerticalScrollBar();
}

ScrollBar* ListBox::getHorizontalScrollBar() const
{
    return viewport->getHorizontalScrollBar();
}

void ListBox::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    viewport->setOpaque (isOpaque());
    repaint();
}

void ListBox::setOutlineThickness (const int outlineThickness_)
{
    outlineThickness = outlineThickness_;
    resized();
}

void ListBox::setHeaderComponent (Component* const newHeaderComponent)
{
    if (headerComponent != newHeaderComponent)
    {
        if (headerComponent != 0)
            delete headerComponent;

        headerComponent = newHeaderComponent;

        addAndMakeVisible (newHeaderComponent);
        ListBox::resized();
    }
}

void ListBox::repaintRow (const int rowNumber)
{
    const Rectangle r (getRowPosition (rowNumber));
    repaint (r.getX(), r.getY(), r.getWidth(), r.getHeight());
}

Image* ListBox::createSnapshotOfSelectedRows()
{
    Image* snapshot = new Image (Image::ARGB, getWidth(), getHeight(), true);
    Graphics g (*snapshot);

    const int firstRow = getRowContainingPosition (0, 0);

    for (int i = getNumRowsOnScreen() + 2; --i >= 0;)
    {
        Component* rowComp = viewport->getComponentForRowIfOnscreen (firstRow + i);

        if (rowComp != 0 && isRowSelected (firstRow + i))
        {
            g.saveState();

            int x = 0, y = 0;
            rowComp->relativePositionToOtherComponent (this, x, y);

            g.setOrigin (x, y);
            g.reduceClipRegion (0, 0, rowComp->getWidth(), rowComp->getHeight());

            rowComp->paintEntireComponent (g);

            g.restoreState();
        }
    }

    return snapshot;
}


//==============================================================================
Component* ListBoxModel::refreshComponentForRow (int, bool, Component* existingComponentToUpdate)
{
    (void) existingComponentToUpdate;
    jassert (existingComponentToUpdate == 0); // indicates a failure in the code the recycles the components
    return 0;
}

void ListBoxModel::listBoxItemClicked (int, const MouseEvent&)
{
}

void ListBoxModel::listBoxItemDoubleClicked (int, const MouseEvent&)
{
}

void ListBoxModel::backgroundClicked()
{
}

void ListBoxModel::selectedRowsChanged (int)
{
}

void ListBoxModel::deleteKeyPressed (int)
{
}

void ListBoxModel::returnKeyPressed (int)
{
}

void ListBoxModel::listWasScrolled()
{
}

const String ListBoxModel::getDragSourceDescription (const SparseSet<int>&)
{
    return String::empty;
}


END_JUCE_NAMESPACE
