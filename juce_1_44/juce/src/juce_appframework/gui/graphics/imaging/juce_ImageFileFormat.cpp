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

#include "juce_ImageFileFormat.h"
#include "../../../../juce_core/io/streams/juce_MemoryInputStream.h"
#include "../../../../juce_core/io/files/juce_FileInputStream.h"
#include "../../../../juce_core/io/streams/juce_BufferedInputStream.h"
#include "image_file_formats/juce_GIFLoader.h"

//==============================================================================
Image* juce_loadPNGImageFromStream (InputStream& inputStream) throw();
bool juce_writePNGImageToStream (const Image& image, OutputStream& out) throw();

PNGImageFormat::PNGImageFormat() throw() {}
PNGImageFormat::~PNGImageFormat() throw() {}

const String PNGImageFormat::getFormatName()
{
    return T("PNG");
}

bool PNGImageFormat::canUnderstand (InputStream& in)
{
    const int bytesNeeded = 4;
    char header [bytesNeeded];

    return in.read (header, bytesNeeded) == bytesNeeded
            && header[1] == 'P'
            && header[2] == 'N'
            && header[3] == 'G';
}

Image* PNGImageFormat::decodeImage (InputStream& in)
{
    return juce_loadPNGImageFromStream (in);
}

bool PNGImageFormat::writeImageToStream (const Image& sourceImage,
                                         OutputStream& destStream)
{
    return juce_writePNGImageToStream (sourceImage, destStream);
}

//==============================================================================
Image* juce_loadJPEGImageFromStream (InputStream& inputStream) throw();
bool juce_writeJPEGImageToStream (const Image& image, OutputStream& out, float quality) throw();

JPEGImageFormat::JPEGImageFormat() throw()
    : quality (-1.0f)
{
}

JPEGImageFormat::~JPEGImageFormat() throw() {}

void JPEGImageFormat::setQuality (const float newQuality)
{
    quality = newQuality;
}

const String JPEGImageFormat::getFormatName()
{
    return T("JPEG");
}

bool JPEGImageFormat::canUnderstand (InputStream& in)
{
    const int bytesNeeded = 10;
    uint8 header [bytesNeeded];

    if (in.read (header, bytesNeeded) == bytesNeeded)
    {
        return header[0] == 0xff
            && header[1] == 0xd8
            && header[2] == 0xff
            && (header[3] == 0xe0 || header[3] == 0xe1);
    }

    return false;
}

Image* JPEGImageFormat::decodeImage (InputStream& in)
{
    return juce_loadJPEGImageFromStream (in);
}

bool JPEGImageFormat::writeImageToStream (const Image& sourceImage,
                                          OutputStream& destStream)
{
    return juce_writeJPEGImageToStream (sourceImage, destStream, quality);
}


//==============================================================================
class GIFImageFormat  : public ImageFileFormat
{
public:
    GIFImageFormat() throw() {}
    ~GIFImageFormat() throw() {}

    const String getFormatName()
    {
        return T("GIF");
    }

    bool canUnderstand (InputStream& in)
    {
        const int bytesNeeded = 4;
        char header [bytesNeeded];

        return (in.read (header, bytesNeeded) == bytesNeeded)
                && header[0] == 'G'
                && header[1] == 'I'
                && header[2] == 'F';
    }

    Image* decodeImage (InputStream& in)
    {
        GIFLoader* const loader = new GIFLoader (in);
        Image* const im = loader->getImage();
        delete loader;
        return im;
    }

    bool writeImageToStream (const Image& /*sourceImage*/, OutputStream& /*destStream*/)
    {
        return false;
    }
};


//==============================================================================
ImageFileFormat* ImageFileFormat::findImageFormatForStream (InputStream& input)
{
    static PNGImageFormat png;
    static JPEGImageFormat jpg;
    static GIFImageFormat gif;

    ImageFileFormat* formats[4];
    int numFormats = 0;

    formats [numFormats++] = &png;
    formats [numFormats++] = &jpg;
    formats [numFormats++] = &gif;

    const int64 streamPos = input.getPosition();

    for (int i = 0; i < numFormats; ++i)
    {
        const bool found = formats[i]->canUnderstand (input);
        input.setPosition (streamPos);

        if (found)
            return formats[i];
    }

    return 0;
}

//==============================================================================
Image* ImageFileFormat::loadFrom (InputStream& input)
{
    ImageFileFormat* const format = findImageFormatForStream (input);

    if (format != 0)
        return format->decodeImage (input);

    return 0;
}

Image* ImageFileFormat::loadFrom (const File& file)
{
    InputStream* const in = file.createInputStream();

    if (in != 0)
    {
        BufferedInputStream b (in, 8192, true);
        return loadFrom (b);
    }

    return 0;
}

Image* ImageFileFormat::loadFrom (const void* rawData, const int numBytes)
{
    if (rawData != 0 && numBytes > 4)
    {
        MemoryInputStream stream (rawData, numBytes, false);
        return loadFrom (stream);
    }

    return 0;
}

END_JUCE_NAMESPACE
