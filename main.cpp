/*******************************************************
 * Copyright (C) 2016  Andrés Solís Montero <andres@solism.ca>
 *   PhD Candidate.
 *   SITE, University of Ottawa
 *   800 King Edward Ave.
 *   Ottawa, On., K1N 6N5, Canada.
 *******************************************************/

#include "viva.h"
#include "factories.h"
#include <sstream>
using namespace viva;


int main(int argc, const char * argv[])
{
    const String keys =
        "{help h            |           | print this message}"
        "{@sequence         |           | url, file, folder, vot_sequence, sequence}"
        "{m method          |skcf       | tracking method: kcf, skcf, ncc, tld, struck, ...}"
        "{p pause           |           | start sequence paused}"
        "{o output          |           | output video filename / folder for images output}"
    ;
    
    CommandLineParser parser(argc, argv, keys);

    if (parser.has("h"))
            parser.printMessage();

    string sequence = parser.get<string>(0);
    string method   = parser.get<string>("m");
    string ofilename   = parser.get<string>("o");
    
    Ptr<Input> input     = TrackerFactory::createInput(sequence);
    Ptr<Tracker> tracker = TrackerFactory::createTracker(method , argc, argv);
    Ptr<Output> output   = TrackerFactory::createOutput(ofilename);
    
    if (parser.has("h") || input.empty() || tracker.empty())
        return 0;
    
    vector<vector<Point2f> > groundTruth;
    TrackerFactory::loadGroundTruth(sequence, groundTruth);
    
    Ptr<ProcessFrame> process = new TrackingProcess(tracker, groundTruth);
    
    Processor processor;
    if (parser.has("p"))
         processor.startPaused();
   
    if (output && parser.has("o"))
        processor.setOutput(output);
    
    processor.setInput(input);
    processor.setProcess(process);
    processor.listenToKeyboardEvents();
    processor.listenToMouseEvents();
    processor.run();

    
    return 0;
}