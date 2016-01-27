/*******************************************************
 * Copyright (C) 2015 Andrés Solís Montero <andres@solism.ca>
 *   PhD Candidate.
 *   SITE, University of Ottawa
 *   800 King Edward Ave.
 *   Ottawa, On., K1N 6N5, Canada.
 *******************************************************/

#include "output.h"


using namespace viva;
ImageOutput::ImageOutput(const string &directory,const Size &size , int suffixSize, int  conversionFlag) :
    Output(size, conversionFlag),_base(directory), _ext(".jpg"),
    _sSize(suffixSize), _internalCount(0), _suffix(0)
{
    if (!Files::exists(_base))
        Files::makeDir(_base);
}

bool ImageOutput::writeFrame(Mat &frame)
{
    std::stringstream ss;
    ss << _base << Files::PATH_SEPARATOR << _suffix <<
        std::setfill('0') << std::setw(_sSize) << _internalCount << _ext;
    if (_convert)
        cvtColor(frame, frame, _conversionFlag);
    if (_size.width > 0 && _size.height > 0)
    {
        resize(frame, frame, _size);
    }
    cv::imwrite(ss.str(), frame);
    _internalCount++;
    return true;

}


void VideoOutput::createOutput()
{
    if (_size.width > 0 && _size.height > 0)
    {
        output.open(_filename, static_cast<int>(_codec), _fps, _size, true);
        _opened = output.isOpened();
        if (!_opened)
        {
            cout << "Video output was not created." << endl;
        }
    }
}

VideoOutput::VideoOutput(const string &filename,
                               Size size,
                               int fps,
                               CODEC codec,
                               int codeFlag ):
    Output(size, codeFlag), _opened(false),
    _fps(fps), _filename(filename), _codec(codec)
{
    createOutput();
}
bool VideoOutput::writeFrame(Mat &frame)
{
	
    
	if (!_opened)
    {
        _size = Size(frame.cols, frame.rows);
        createOutput();
    }
    
    Mat tmp;
	if (_size != frame.size())
		resize(frame, frame, _size);
    if (_convert)
        cvtColor(frame, frame, _conversionFlag);
	
	output << frame;
	return _opened;
}

