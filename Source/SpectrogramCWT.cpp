/*
  ==============================================================================

    SpectrogramCWT.cpp
    Created: 23 May 2021 3:20:19pm
    Author:  menin

  ==============================================================================
*/

#include "SpectrogramCWT.h"

ColourPos::ColourPos()
{
    position = 0;
}

ColourPos::ColourPos(Colour _color, double pos)
{
    color = _color;
    position = pos;
}

ColourPos::~ColourPos()
{
}


Palete::Palete()
{
}

Palete::~Palete()
{
    _colors.clear();
    _colors.shrink_to_fit();
}

void Palete::AddColour(ColourPos color)
{
    _colors.push_back(color);
    std::sort(_colors.begin(), _colors.end(), [](const ColourPos& a, const ColourPos& b) -> bool
        {
            return a.position > b.position;
        }
    );
}

Colour Palete::GetColour(double value)
{
    if (value < _min || value > _max) return blancColour;

    auto range = _max - _min;
    auto curPos = value - _min;
    auto procPos = curPos / range;

    ColourPos curentColour;
    ColourPos prevColour;

    for (int i = 0; i < _colors.size(); ++i) 
    {
        curentColour = _colors.at(i);
        if (curentColour.position == procPos) return curentColour.color;

        if (curentColour.position < procPos) 
        {
            if (i == 0) {
                throw new Expression();
            }
            prevColour = _colors.at(i - 1);

            auto rangeCol = curentColour.position - prevColour.position;
            auto curPosCol = procPos - prevColour.position;
            auto procPosColourPrev = curPosCol / rangeCol;

            auto procPosColourCurent = 1.0 - procPosColourPrev;

            return Colour(
                prevColour.color.getRed() * procPosColourPrev + curentColour.color.getRed() * procPosColourCurent,
                prevColour.color.getGreen() * procPosColourPrev + curentColour.color.getGreen() * procPosColourCurent,
                prevColour.color.getBlue() * procPosColourPrev + curentColour.color.getBlue() * procPosColourCurent
            );
        }
    }

    return blancColour;
}


SpectrogramCWT::SpectrogramCWT():
    _wt(nullptr),
    _spectre(nullptr)
{
    _palete.AddColour(ColourPos(Colours::black, 0));
    _palete.AddColour(ColourPos(Colours::blue.darker(), 0.1));
    _palete.AddColour(ColourPos(Colours::brown, 0.2));
    _palete.AddColour(ColourPos(Colours::coral.darker(), 0.3));
    _palete.AddColour(ColourPos(Colours::darkcyan, 0.4));
    _palete.AddColour(ColourPos(Colours::green.darker(), 0.5));
    _palete.AddColour(ColourPos(Colours::green, 0.6));
    _palete.AddColour(ColourPos(Colours::yellowgreen, 0.7));
    _palete.AddColour(ColourPos(Colours::blue, 0.8));
    _palete.AddColour(ColourPos(Colours::yellow, 0.9));
    _palete.AddColour(ColourPos(Colours::red, 1.0));

    //_palete.AddColour(ColourPos(Colours::brown, 0.25));
    //_palete.AddColour(ColourPos(Colours::yellow, 0.5));
    //_palete.AddColour(ColourPos(Colours::green, 0.75));
    //_palete.AddColour(ColourPos(Colours::red, 1));
}

SpectrogramCWT::~SpectrogramCWT()
{
}

void SpectrogramCWT::paint(juce::Graphics& g)
{
    g.drawImage(mBufferImage, Rectangle<float>(0, 0, getWidth(), getHeight()));
}

void SpectrogramCWT::resized()
{
    mBufferImage = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    DrawOnImageFFT_TREE(mBufferImage);
}

void SpectrogramCWT::SetSpectre(cwt_object wt)
{
    _wt = wt;
    dataLoaded = true;
    DrawOnImageCWT(mBufferImage);
}

void SpectrogramCWT::SetSpectre(FFT_Tree_Spectre* data)
{
    _spectre = data;
    dataLoaded = true;
    DrawOnImageFFT_TREE(mBufferImage);
}

void SpectrogramCWT::Clear()
{
    _spectre = nullptr;
    dataLoaded = false;
    DrawOnImageFFT_TREE(mBufferImage);
}


std::tuple<double, double> SpectrogramCWT::GetMinMax(cplx_data* massive, int size)
{
    double min = INFINITY;
    double max = -INFINITY;
    double value;
    cplx_data item;

    for (int i = 0; i < size; ++i)
    {
        item = massive[i];
        value = hypot(item.re, item.im);

        if (min > value) {
            min = value;
        }        
        if (max < value) {
            max = value;
        }
    }
    return { min, max };
}

std::tuple<double, double> SpectrogramCWT::GetMinMax(double* massive, int size)
{
    double min = INFINITY;
    double max = -INFINITY;
    double value;

    for (int i = 0; i < size; ++i)
    {
        value = massive[i];

        if (min > value) {
            min = value;
        }
        if (max < value) {
            max = value;
        }
    }
    return { min, max };
}

void SpectrogramCWT::DrawOnImageCWT(Image& image)
{
    Graphics g(image);
    g.fillAll(backgroundColour);
    if (!dataLoaded) return;

    auto massive = _wt->output;
    auto size = _wt->siglength;
    auto J = _wt->J;

    double min, max;
    std::tie(min, max) = GetMinMax(massive, size*J);

    _palete.SetMax(max);
    _palete.SetMin(min);

    int freq, curTime;
    int index;

    int width = getWidth();
    int height = getHeight();

    double scaleY = double(J) / double(height);
    double scaleX = double(size) / double(width);
    
    cplx_data item;
    double value;

    //for(int i = 0; i < width; ++i)
    //    for (int j = 0; j < height; ++j) {
    //        freq = trunc(j * scaleY);
    //        curTime = trunc(i * scaleX);
    //        index = freq * size + curTime;

    //        item = massive[index];
    //        value = hypot(item.re, item.im);

    //        g.setColour(_palete.GetColour(value));
    //        g.fillRect(i, j, 1, 1);
    //    }

    //generate Real Image

    mRealImage = Image(Image::PixelFormat::RGB, size, J, true);
    Graphics realG(mRealImage);
    for (int i = 0; i < size; ++i)
    {
        for (int k = 0; k < J; ++k) 
        {
            auto index = k * size + i;
            auto item = massive[index];

            value = hypot(item.re, item.im);
            realG.setColour(_palete.GetColour(value));
            realG.fillRect(i, k, 1, 1);
        }
    }

    g.drawImage(mRealImage, 0, 0, width, height, 0, 0, size, J);

    std::string path = "D:\\YandexDisk\\imageCWT";
    std::string ext = ".png";
    auto name = path + ext;
    auto file = File(name);
    int counter = 1;
    //save Real Image
    do 
    {
        if (file.exists()) {
            file = File(path + std::to_string(counter) + ext);
            ++counter;
        }
        else break;
    } while (true);

    FileOutputStream stream(file);
    JPEGImageFormat jpgWriter;
    PNGImageFormat pngWriter;
    //jpgWriter.writeImageToStream(mRealImage, stream);
    bool answer = pngWriter.writeImageToStream(mRealImage, stream);
}

void SpectrogramCWT::DrawOnImageFFT_TREE(Image& image)
{
    Graphics g(image);
    g.fillAll(backgroundColour);
    if (!dataLoaded) return;

    auto massive = _spectre->spectre;
    auto size = _spectre->numBins;
    auto J = _spectre->numFrequeance /10;

    double min, max;
    std::tie(min, max) = GetMinMax(massive, _spectre->size);

    _palete.SetMax(max);
    _palete.SetMin(min);

    int freq, curTime;
    int index;

    int width = getWidth();
    int height = getHeight();


    //cplx_data item;
    double value;

    //generate Real Image

    mRealImage = Image(Image::PixelFormat::RGB, size, J, true);
    Graphics realG(mRealImage);
    for (int i = 0; i < size; ++i)
    {
        for (int k = 0; k < J; ++k)
        {
            realG.setColour(_palete.GetColour(_spectre->GetValue(i, k)));
            realG.fillRect(i, k, 1, 1);
        }
    }

    g.drawImage(mRealImage, 0, 0, width, height, 0, 0, size, J);

    //save Real Image

    if(true){
        std::string path = "D:\\YandexDisk\\imageFFT_TREE";
        std::string ext = ".png";
        auto name = path + ext;
        auto file = File(name);
        int counter = 1;

        do
        {
            if (file.exists()) {
                file = File(path + std::to_string(counter) + ext);
                ++counter;
            }
            else break;
        } while (true);

        FileOutputStream stream(file);
        PNGImageFormat pngWriter;
        bool answer = pngWriter.writeImageToStream(mRealImage, stream);
    }
}
