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

#include "../../jucer_Headers.h"
#include "jucer_FillType.h"


//==============================================================================
FillType::FillType()
    : image (0)
{
    reset();
}

FillType::FillType (const FillType& other)
{
    image = 0;
    mode = other.mode;
    colour = other.colour;
    gradCol1 = other.gradCol1;
    gradCol2 = other.gradCol2;
    gradPos1 = other.gradPos1;
    gradPos2 = other.gradPos2;
    imageResourceName = other.imageResourceName;
    imageOpacity = other.imageOpacity;
    imageAnchor = other.imageAnchor;
}

const FillType& FillType::operator= (const FillType& other)
{
    ImageCache::release (image);
    image = 0;

    mode = other.mode;
    colour = other.colour;
    gradCol1 = other.gradCol1;
    gradCol2 = other.gradCol2;
    gradPos1 = other.gradPos1;
    gradPos2 = other.gradPos2;
    imageResourceName = other.imageResourceName;
    imageOpacity = other.imageOpacity;
    imageAnchor = other.imageAnchor;

    return *this;
}

FillType::~FillType()
{
    ImageCache::release (image);
}

bool FillType::operator== (const FillType& other) const throw()
{
    return mode == other.mode
        && colour == other.colour
        && gradCol1 == other.gradCol1
        && gradCol2 == other.gradCol2
        && gradPos1 == other.gradPos1
        && gradPos2 == other.gradPos2
        && imageResourceName == other.imageResourceName
        && imageOpacity == other.imageOpacity
        && imageAnchor == other.imageAnchor;
}

bool FillType::operator!= (const FillType& other) const throw()
{
    return ! operator== (other);
}

void FillType::reset()
{
    ImageCache::release (image);
    image = 0;

    mode = solidColour;
    colour = Colours::brown.withHue (Random::getSystemRandom().nextFloat());
    gradCol1 = Colours::red;
    gradCol2 = Colours::green;
    gradPos1 = RelativePositionedRectangle();
    gradPos1.rect = PositionedRectangle (T("50 50"));
    gradPos2 = RelativePositionedRectangle();
    gradPos2.rect = PositionedRectangle (T("100 100"));

    imageResourceName = String::empty;
    imageOpacity = 1.0;
    imageAnchor = RelativePositionedRectangle();
    imageAnchor.rect = PositionedRectangle (T("0 0"));
}

//==============================================================================
Brush* FillType::createBrush (JucerDocument* const document,
                              const Rectangle& parentArea)
{
    if (mode == solidColour)
    {
        ImageCache::release (image);
        image = 0;

        return new SolidColourBrush (colour);
    }
    else if (mode == imageBrush)
    {
        loadImage (document);

        Rectangle r (imageAnchor.getRectangle (parentArea, document->getComponentLayout()));

        return new ImageBrush (image, r.getX(), r.getY(), (float) imageOpacity);
    }
    else
    {
        ImageCache::release (image);
        image = 0;

        Rectangle r1 (gradPos1.getRectangle (parentArea, document->getComponentLayout()));
        Rectangle r2 (gradPos2.getRectangle (parentArea, document->getComponentLayout()));

        return new GradientBrush (gradCol1, (float) r1.getX(), (float) r1.getY(),
                                  gradCol2, (float) r2.getX(), (float) r2.getY(),
                                  mode == radialGradient);
    }
}

void FillType::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const
{
    String s;

    switch (mode)
    {
    case solidColour:
        s << "g.setColour (" << colourToCode (colour) << ");\n";
        break;

    case linearGradient:
    case radialGradient:
        {
            String x1, y1, w, h, x2, y2;
            positionToCode (gradPos1, code.document->getComponentLayout(), x1, y1, w, h);
            positionToCode (gradPos2, code.document->getComponentLayout(), x2, y2, w, h);

            const int brushNum = code.getUniqueSuffix();

            s << "GradientBrush gradient_" << brushNum << " (";

            const String indent (String::repeatedString (T(" "), s.length()));

            s << colourToCode (gradCol1) << ",\n"
              << indent << castToFloat (x1) << ", " << castToFloat (y1) << ",\n"
              << indent << colourToCode (gradCol2) << ",\n"
              << indent << castToFloat (x2) << ", " << castToFloat (y2) << ",\n"
              << indent << boolToString (mode == radialGradient) << ");\n"
              << "g.setBrush (&gradient_" << brushNum << ");\n";
            break;
        }

    case imageBrush:
        {
            const String imageVariable (T("internalCachedImage") + String (code.getUniqueSuffix()));

            code.addImageResourceLoader (imageVariable, imageResourceName);

            String x, y, w, h;
            positionToCode (imageAnchor, code.document->getComponentLayout(), x, y, w, h);

            const int brushNum = code.getUniqueSuffix();

            s << "ImageBrush imageBrush_" << brushNum << " (";

            const String indent (String::repeatedString (T(" "), s.length()));

            s << imageVariable << ",\n"
              << indent << x << ", " << y << ",\n"
              << indent << valueToFloat (imageOpacity) << ");\n"
              << "g.setBrush (&imageBrush_" << brushNum << ");\n";
            break;
        }

    default:
        jassertfalse
        break;
    }

    paintMethodCode += s;
}

const String FillType::toString() const
{
    switch (mode)
    {
    case solidColour:
        return T("solid: ") + colourToHex (colour);

    case linearGradient:
    case radialGradient:
        return (mode == linearGradient ? T("linear: ")
                                       : T(" radial: "))
                + positionToString (gradPos1)
                + T(", ")
                + positionToString (gradPos2)
                + T(", 0=") + colourToHex (gradCol1)
                + T(", 1=") + colourToHex (gradCol2);

    case imageBrush:
        return T("image: ") + imageResourceName
                + T(", ")
                + String (imageOpacity)
                + T(", ")
                + positionToString (imageAnchor);

    default:
        jassertfalse
        break;
    }

    return String::empty;
}

void FillType::restoreFromString (const String& s)
{
    reset();

    if (s.isNotEmpty())
    {
        StringArray toks;
        toks.addTokens (s, T(",:"), 0);
        toks.trim();

        if (toks[0] == T("solid"))
        {
            mode = solidColour;
            colour = Colour (toks[1].getHexValue32());
        }
        else if (toks[0] == T("linear")
                  || toks[0] == T("radial"))
        {
            mode = (toks[0] == T("linear")) ? linearGradient : radialGradient;

            gradPos1 = RelativePositionedRectangle();
            gradPos1.rect = PositionedRectangle (toks[1]);
            gradPos2 = RelativePositionedRectangle();
            gradPos2.rect = PositionedRectangle (toks[2]);

            gradCol1 = Colour (toks[3].fromFirstOccurrenceOf (T("="), false, false).getHexValue32());
            gradCol2 = Colour (toks[4].fromFirstOccurrenceOf (T("="), false, false).getHexValue32());
        }
        else if (toks[0] == T("image"))
        {
            mode = imageBrush;
            imageResourceName = toks[1];
            imageOpacity = toks[2].getDoubleValue();
            imageAnchor= RelativePositionedRectangle();
            imageAnchor.rect = PositionedRectangle (toks[3]);
        }
        else
        {
            jassertfalse
        }
    }
}

bool FillType::isOpaque() const
{
    switch (mode)
    {
    case solidColour:
        return colour.isOpaque();

    case linearGradient:
    case radialGradient:
        return gradCol1.isOpaque() && gradCol1.isOpaque();

    case imageBrush:
        return image != 0
                 && imageOpacity >= 1.0f
                 && ! image->hasAlphaChannel();

    default:
        jassertfalse
        break;
    }

    return false;
}

bool FillType::isInvisible() const
{
    switch (mode)
    {
    case solidColour:
        return colour.isTransparent();

    case linearGradient:
    case radialGradient:
        return gradCol1.isTransparent() && gradCol2.isTransparent();

    case imageBrush:
        return imageOpacity == 0;

    default:
        jassertfalse
        break;
    }

    return false;
}

void FillType::loadImage (JucerDocument* const document)
{
    if (image == 0)
    {
        if (document != 0)
            image = document->getResources().getImageFromCache (imageResourceName);

        if (image == 0)
        {
            const int hashCode = 0x3437856f;
            image = ImageCache::getFromHashCode (hashCode);

            if (image == 0)
            {
                image = new Image (Image::RGB, 100, 100, true);

                Graphics g (*image);
                g.fillCheckerBoard (0, 0, image->getWidth(), image->getHeight(),
                                    image->getWidth() / 2, image->getHeight() / 2,
                                    Colours::white, Colours::lightgrey);

                g.setFont (12.0f);
                g.setColour (Colours::grey);
                g.drawText (T("(image missing)"), 0, 0, image->getWidth(), image->getHeight() / 2, Justification::centred, true);

                ImageCache::addImageToCache (image, hashCode);
            }
        }
    }
}
