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

#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__

#include "juce_LowLevelGraphicsContext.h"


//==============================================================================
/**
    An implementation of LowLevelGraphicsContext that turns the drawing operations
    into a PostScript document.

*/
class JUCE_API  LowLevelGraphicsPostScriptRenderer    : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                        const String& documentTitle,
                                        const int totalWidth,
                                        const int totalHeight);

    ~LowLevelGraphicsPostScriptRenderer();

    //==============================================================================
    bool isVectorDevice() const;
    void setOrigin (int x, int y);

    bool reduceClipRegion (int x, int y, int w, int h);
    bool reduceClipRegion (const RectangleList& clipRegion);
    void excludeClipRegion (int x, int y, int w, int h);

    void saveState();
    void restoreState();

    bool clipRegionIntersects (int x, int y, int w, int h);
    const Rectangle getClipBounds() const;
    bool isClipEmpty() const;

    //==============================================================================
    void fillRectWithColour (int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents);
    void fillRectWithGradient (int x, int y, int w, int h, const ColourGradient& gradient);

    void fillPathWithColour (const Path& path, const AffineTransform& transform, const Colour& colour, EdgeTable::OversamplingLevel quality);
    void fillPathWithGradient (const Path& path, const AffineTransform& transform, const ColourGradient& gradient, EdgeTable::OversamplingLevel quality);
    void fillPathWithImage (const Path& path, const AffineTransform& transform,
                            const Image& image, int imageX, int imageY, float alpha, EdgeTable::OversamplingLevel quality);

    void fillAlphaChannelWithColour (const Image& alphaImage, int imageX, int imageY, const Colour& colour);
    void fillAlphaChannelWithGradient (const Image& alphaImage, int imageX, int imageY, const ColourGradient& gradient);
    void fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                    const Image& fillerImage, int fillerImageX, int fillerImageY, float alpha);

    //==============================================================================
    void blendImage (const Image& sourceImage, int destX, int destY, int destW, int destH,
                     int sourceX, int sourceY, float alpha);

    void blendImageRescaling (const Image& sourceImage, int destX, int destY, int destW, int destH,
                              int sourceX, int sourceY, int sourceW, int sourceH,
                              float alpha, const Graphics::ResamplingQuality quality);

    void blendImageWarping (const Image& sourceImage, int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                            const AffineTransform& transform,
                            float alpha, const Graphics::ResamplingQuality quality);

    //==============================================================================
    void drawLine (double x1, double y1, double x2, double y2, const Colour& colour);

    void drawVerticalLine (const int x, double top, double bottom, const Colour& col);
    void drawHorizontalLine (const int x, double top, double bottom, const Colour& col);


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    OutputStream& out;
    RectangleList* clip;
    int totalWidth, totalHeight, xOffset, yOffset;
    bool needToClip;
    Colour lastColour;

    struct SavedState
    {
        SavedState (RectangleList* const clip, const int xOffset, const int yOffset);
        ~SavedState();

        RectangleList* clip;
        const int xOffset, yOffset;

    private:
        SavedState (const SavedState&);
        const SavedState& operator= (const SavedState&);
    };

    OwnedArray <SavedState> stateStack;

    void writeClip();
    void writeColour (const Colour& colour);
    void writePath (const Path& path) const;
    void writeXY (const float x, const float y) const;
    void writeTransform (const AffineTransform& trans) const;
    void writeImage (const Image& im, const int sx, const int sy, const int maxW, const int maxH) const;

    LowLevelGraphicsPostScriptRenderer (const LowLevelGraphicsPostScriptRenderer& other);
    const LowLevelGraphicsPostScriptRenderer& operator= (const LowLevelGraphicsPostScriptRenderer&);
};



#endif   // __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
