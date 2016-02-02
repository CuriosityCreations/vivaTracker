/*******************************************************
 * Copyright (C) 2016  Andrés Solís Montero <andres@solism.ca>
 *   PhD Candidate.
 *   SITE, University of Ottawa
 *   800 King Edward Ave.
 *   Ottawa, On., K1N 6N5, Canada.
 *******************************************************/

#include "factories.h"


string TrackerFactory::SEQ_BASE_FILE     = "sequences.txt";
string TrackerFactory::GROUND_TRUTH_FILE = "groundtruth.txt";

void GroundTruth::parse(const string file, vector<vector<Point2f> > &gt)
{
    gt.clear();
    
    if (viva::Files::isFile(file))
    {
        std::ifstream infile(file);
        std::string line, base;
        while (std::getline(infile, line))
        {
            vector<float> values;
            split<float>(line, ',', values);
            vector<Point2f> points;
            
            if (values.size() == 4)
            {
                points = {
                    Point2f(values[0], values[1]),
                    Point2f(values[0] + values[2], values[1]),
                    Point2f(values[0] + values[2], values[1] + values[3]),
                    Point2f(values[0], values[1]+ values[3]) };
            }
            if (values.size() == 8)
            {
                points = {
                    Point2f(values[0], values[1]),
                    Point2f(values[2], values[3]),
                    Point2f(values[4], values[5]),
                    Point2f(values[6], values[7]) };
            }
            
            gt.push_back(points);
            
        }
        infile.close();
    }
}


bool TrackerFactory::isVideoFile(const string &sequence)
{
    return (viva::Files::isFile(sequence));
}
bool TrackerFactory::isWebFile(const string &sequence)
{
    return sequence.substr(0,4) == "http";
}
bool TrackerFactory::isStringSequence(const string &sequence)
{
    char res0[sequence.length() * 2];
    char res1[sequence.length() * 2];
    sprintf(res0, sequence.c_str(), 0);
    sprintf(res1, sequence.c_str(), 1);
    string path0(res0);
    string path1(res1);
    return (viva::Files::isFile(path0) || viva::Files::isFile(path1));
}
bool TrackerFactory::isFolderSequence(const string &sequence)
{
    return (viva::Files::isDir(sequence));
}
string TrackerFactory::constructSequenceFolder(const string &file, const string &sequence)
{
    std::ifstream infile(file);
    std::string line, base;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        iss >> base;
        break;
    }
    infile.close();
    base = (base.length() == 0)? ("." + viva::Files::PATH_SEPARATOR) : base;
    ostringstream ss;
    ss << base << sequence << viva::Files::PATH_SEPARATOR;
    return ss.str();
}



Ptr<Input> TrackerFactory::createInput(const string &sequence)
{
    if (isVideoFile(sequence))
    {
        return new VideoInput(sequence);
    }
    if (isWebFile(sequence))
    {
        return new VideoInput(sequence);
        
    }
    if (isStringSequence(sequence))
    {
        return new VideoInput(sequence);
    }
    
    if (isFolderSequence(sequence))
    {
        return new ImageListInput(sequence, Size(-1,-1), -1, 0);
    }
    string path = constructSequenceFolder(SEQ_BASE_FILE, sequence);
    if (isFolderSequence(path))
    {
        return new ImageListInput(path, Size(-1,-1), -1, 0);
    }
    return Ptr<Input>();
}

Ptr<Output> TrackerFactory::createOutput(const string &filename)
{
    string extension;
    viva::Files::getExtension(filename, extension);
    
    if (viva::Files::isDir(filename))
        return new ImageOutput(filename);
    else if (extension != "")
        return new VideoOutput(filename);
    
    return Ptr<Output>();
}


Ptr<Tracker> TrackerFactory::createTracker(const string &method, const int argc, const char * argv[])
{
    Ptr<Tracker> tracker;
    argv[0] = method.c_str();
    
    if (method == "skcf")
    {
        const String keys =
        "{? usage           |       | print this message}"
        "{t type            |g      | correlation type: g(gaussian), p(polynomial), l(linear)}"
        "{f feat            |fhog   | feature type: fhog, gray, rgb, hsv, hls}"
        "{s scale           |       | turn on scale estimation}";
        
        CommandLineParser parser(argc, argv, keys);
        
        if (parser.has("?"))
        {
            parser.printMessage();
            return tracker;
        }
        
        KType type;
        KFeat feat;
        
        if (parser.get<string>("t") == "l")
            type=KType::LINEAR;
        else if(parser.get<string>("t") == "p")
            type=KType::POLYNOMIAL;
        else
            type=KType::GAUSSIAN;
        
        if (parser.get<string>("f") == "gray")
            feat = KFeat::GRAY;
        else if (parser.get<string>("f") == "rgb")
            feat = KFeat::RGB;
        else if (parser.get<string>("f") == "hsv")
            feat = KFeat::HSV;
        else if (parser.get<string>("f") == "hls")
            feat = KFeat::HLS;
        else
            feat = KFeat::FHOG;
        
        tracker = new SKCFDCF(KType::GAUSSIAN, KFeat::FHOG, parser.has("s"));
    }
    if (method == "ncc")
    {
        const String keys =
        "{? usage           |       | print this message}";
        
        CommandLineParser parser(argc, argv, keys);
        
        if (parser.has("?"))
        {
            parser.printMessage();
            return tracker;
        }
        tracker = new NCCTracker();
    }
    if (method == "kcf")
    {
        const String keys =
        "{? usage           |       | print this message}"
        "{f feat            |hog    | feature type: hog, lab, gray}"
        "{s scale           |       | turn on scale estimation}";
        
        CommandLineParser parser(argc, argv, keys);
        
        if (parser.has("?"))
        {
            parser.printMessage();
            return tracker;
        }
        
        bool HOG = true;
        bool FIXEDWINDOW = false;
        bool MULTISCALE = false;
        bool LAB = false;
        
        if (parser.has("s"))
            MULTISCALE = true;
        if (parser.get<string>("f") == "hog")
        {
            HOG = true;
        }
        else if (parser.get<string>("f") == "lab")
        {
            HOG = true;
            LAB = true;
        }
        else if (parser.get<string>("f") == "gray")
        {
            HOG = false;
        }
        
        tracker = new KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
    }
    if (method == "struck")
    {
        const String keys =
        "{? usage           |       | print this message}";
        
        CommandLineParser parser(argc, argv, keys);
        
        if (parser.has("?"))
        {
            parser.printMessage();
            return tracker;
        }
        tracker = new STRUCKtracker();
    }
    if (method == "tld")
    {
        const String keys =
        "{? usage           |       | print this message}";
        
        CommandLineParser parser(argc, argv, keys);
        
        if (parser.has("?"))
        {
            parser.printMessage();
            return tracker;
        }
        tracker = new OpenTLD();
    }
    

    return tracker;
}
void TrackerFactory::loadGroundTruth(const string &sequence, vector<vector<Point2f> > &groundTruth)
{
    string basename;
    if (isVideoFile(sequence))
    {
        viva::Files::getBasename(sequence, basename);
    }
    if (isWebFile(sequence))
    {
        //TODO
    }
    if (isStringSequence(sequence))
    {
        viva::Files::getBasename(sequence, basename);
    }
    
    if (isFolderSequence(sequence))
    {
        basename = sequence;
    }
    string path = constructSequenceFolder(SEQ_BASE_FILE, sequence);
    if (isFolderSequence(path))
    {
        basename = path;
    }
    
    GroundTruth::parse(basename + GROUND_TRUTH_FILE, groundTruth);
}

