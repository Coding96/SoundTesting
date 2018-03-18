

#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>

#include <iostream>
#include <fstream>
#include <set>
#include <sndfile.h>
#include <vector>
#include <math.h>
#include <cmath>
#include <chrono>
#include <cstring>
#include <cstdlib>

#include "system.h"
#include "event.h"


#define DEG_TO_RAD 0.017453293

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;


//function declarations for glut
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void animate(void);
void initializeGraphics(void);
void menu(int i);
void calculate_lookpoint(void);
void createEvents();

enum Verbosity
{ //an enum type to specify output of plugin
    PluginIds,
    PluginOutputIds,
    PluginOutput,
    PluginInformation,
    PluginInformationDetailed
};

//global variables for tracking movement of player

GLfloat lon, lat;
GLfloat centerx, centery, centerz;
GLfloat eyex, eyey, eyez;
GLfloat upx, upy, upz;

//vector for holding events
std::vector<event> eventVector;
float currentTime;
//example function to be delted later

string header(string text, int level)
{
    string out = '\n' + text + '\n';
    for (size_t i = 0; i < text.length(); ++i)
    {
        out += (level == 1 ? '=' : level == 2 ? '-' : '~');
    }
    out += '\n';
    return out;
}

//utility function converts type RealTime to a floating value in seconds

static double toSeconds(const RealTime &time)
{
    return time.sec + double(time.nsec + 1) / 1000000000.0;
}


//example function to modifiy it is called when runPlugin is finished
//needs to be modified to store results in a data structure

int printFeatures(int frame, int sr,
                  const Plugin::OutputDescriptor &output, int outputNo,
                  const Plugin::FeatureSet &features, ofstream *out, bool useFrames)
{
    static int featureCount = -1;

    if (features.find(outputNo) == features.end()) return 0;

    for (size_t i = 0; i < features.at(outputNo).size(); ++i)
    {

        const Plugin::Feature &f = features.at(outputNo).at(i);

        bool haveRt = false;
        RealTime rt;

        if (output.sampleType == Plugin::OutputDescriptor::VariableSampleRate)
        {
            rt = f.timestamp;
            haveRt = true;
        }
        else if (output.sampleType == Plugin::OutputDescriptor::FixedSampleRate)
        {
            int n = featureCount + 1;
            if (f.hasTimestamp)
            {
                n = int(round(toSeconds(f.timestamp) * output.sampleRate));
            }
            rt = RealTime::fromSeconds(double(n) / output.sampleRate);
            haveRt = true;
            featureCount = n;
        }

        if (useFrames)
        {

            int displayFrame = frame;

            if (haveRt)
            {
                displayFrame = RealTime::realTime2Frame(rt, sr);
            }

            (out ? *out : cout) << displayFrame;

            if (f.hasDuration)
            {
                displayFrame = RealTime::realTime2Frame(f.duration, sr);
                (out ? *out : cout) << "," << displayFrame;
            }

            (out ? *out : cout) << ":";

        }
        else
        {

            if (!haveRt)
            {
                rt = RealTime::frame2RealTime(frame, sr);
            }

            (out ? *out : cout) << rt.toString();

            if (f.hasDuration)
            {
                rt = f.duration;
                (out ? *out : cout) << "," << rt.toString();
            }

            (out ? *out : cout) << ":";
        }

        for (unsigned int j = 0; j < f.values.size(); ++j)
        {
            (out ? *out : cout) << " " << f.values[j];
        }
        (out ? *out : cout) << " " << f.label;

        (out ? *out : cout) << endl;
    }
}

//example function to be deleted later

void enumeratePlugins(Verbosity verbosity)
{
    PluginLoader *loader = PluginLoader::getInstance();

    if (verbosity == PluginInformation)
    {
        cout << "\nVamp plugin libraries found in search path:" << endl;
    }

    vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    typedef multimap<string, PluginLoader::PluginKey>
            LibraryMap;
    LibraryMap libraryMap;

    for (size_t i = 0; i < plugins.size(); ++i)
    {
        string path = loader->getLibraryPathForPlugin(plugins[i]);
        libraryMap.insert(LibraryMap::value_type(path, plugins[i]));
    }

    string prevPath = "";
    int index = 0;

    for (LibraryMap::iterator i = libraryMap.begin();
            i != libraryMap.end(); ++i)
    {

        string path = i->first;
        PluginLoader::PluginKey key = i->second;

        if (path != prevPath)
        {
            prevPath = path;
            index = 0;
            if (verbosity == PluginInformation)
            {
                cout << "\n  " << path << ":" << endl;
            }
            else if (verbosity == PluginInformationDetailed)
            {
                string::size_type ki = i->second.find(':');
                string text = "Library \"" + i->second.substr(0, ki) + "\"";
                cout << "\n" << header(text, 1);
            }
        }

        Plugin *plugin = loader->loadPlugin(key, 48000);
        if (plugin)
        {

            char c = char('A' + index);
            if (c > 'Z') c = char('a' + (index - 26));

            PluginLoader::PluginCategoryHierarchy category =
                    loader->getPluginCategory(key);
            string catstr;
            if (!category.empty())
            {
                for (size_t ci = 0; ci < category.size(); ++ci)
                {
                    if (ci > 0) catstr += " > ";
                    catstr += category[ci];
                }
            }

            if (verbosity == PluginInformation)
            {

                cout << "    [" << c << "] [v"
                        << plugin->getVampApiVersion() << "] "
                        << plugin->getName() << ", \""
                        << plugin->getIdentifier() << "\"" << " ["
                        << plugin->getMaker() << "]" << endl;

                if (catstr != "")
                {
                    cout << "       > " << catstr << endl;
                }

                if (plugin->getDescription() != "")
                {
                    cout << "        - " << plugin->getDescription() << endl;
                }

            }
            else if (verbosity == PluginInformationDetailed)
            {

                cout << header(plugin->getName(), 2);
                cout << " - Identifier:         "
                        << key << endl;
                cout << " - Plugin Version:     "
                        << plugin->getPluginVersion() << endl;
                cout << " - Vamp API Version:   "
                        << plugin->getVampApiVersion() << endl;
                cout << " - Maker:              \""
                        << plugin->getMaker() << "\"" << endl;
                cout << " - Copyright:          \""
                        << plugin->getCopyright() << "\"" << endl;
                cout << " - Description:        \""
                        << plugin->getDescription() << "\"" << endl;
                cout << " - Input Domain:       "
                        << (plugin->getInputDomain() == Vamp::Plugin::TimeDomain ?
                        "Time Domain" : "Frequency Domain") << endl;
                cout << " - Default Step Size:  "
                        << plugin->getPreferredStepSize() << endl;
                cout << " - Default Block Size: "
                        << plugin->getPreferredBlockSize() << endl;
                cout << " - Minimum Channels:   "
                        << plugin->getMinChannelCount() << endl;
                cout << " - Maximum Channels:   "
                        << plugin->getMaxChannelCount() << endl;

            }
            else if (verbosity == PluginIds)
            {
                cout << "vamp:" << key << endl;
            }

            Plugin::OutputList outputs =
                    plugin->getOutputDescriptors();

            if (verbosity == PluginInformationDetailed)
            {

                Plugin::ParameterList params = plugin->getParameterDescriptors();
                for (size_t j = 0; j < params.size(); ++j)
                {
                    Plugin::ParameterDescriptor & pd(params[j]);
                    cout << "\nParameter " << j + 1 << ": \"" << pd.name << "\"" << endl;
                    cout << " - Identifier:         " << pd.identifier << endl;
                    cout << " - Description:        \"" << pd.description << "\"" << endl;
                    if (pd.unit != "")
                    {
                        cout << " - Unit:               " << pd.unit << endl;
                    }
                    cout << " - Range:              ";
                    cout << pd.minValue << " -> " << pd.maxValue << endl;
                    cout << " - Default:            ";
                    cout << pd.defaultValue << endl;
                    if (pd.isQuantized)
                    {
                        cout << " - Quantize Step:      "
                                << pd.quantizeStep << endl;
                    }
                    if (!pd.valueNames.empty())
                    {
                        cout << " - Value Names:        ";
                        for (size_t k = 0; k < pd.valueNames.size(); ++k)
                        {
                            if (k > 0) cout << ", ";
                            cout << "\"" << pd.valueNames[k] << "\"";
                        }
                        cout << endl;
                    }
                }

                if (outputs.empty())
                {
                    cout << "\n** Note: This plugin reports no outputs!" << endl;
                }
                for (size_t j = 0; j < outputs.size(); ++j)
                {
                    Plugin::OutputDescriptor & od(outputs[j]);
                    cout << "\nOutput " << j + 1 << ": \"" << od.name << "\"" << endl;
                    cout << " - Identifier:         " << od.identifier << endl;
                    cout << " - Description:        \"" << od.description << "\"" << endl;
                    if (od.unit != "")
                    {
                        cout << " - Unit:               " << od.unit << endl;
                    }
                    if (od.hasFixedBinCount)
                    {
                        cout << " - Default Bin Count:  " << od.binCount << endl;
                    }
                    if (!od.binNames.empty())
                    {
                        bool have = false;
                        for (size_t k = 0; k < od.binNames.size(); ++k)
                        {
                            if (od.binNames[k] != "")
                            {
                                have = true;
                                break;
                            }
                        }
                        if (have)
                        {
                            cout << " - Bin Names:          ";
                            for (size_t k = 0; k < od.binNames.size(); ++k)
                            {
                                if (k > 0) cout << ", ";
                                cout << "\"" << od.binNames[k] << "\"";
                            }
                            cout << endl;
                        }
                    }
                    if (od.hasKnownExtents)
                    {
                        cout << " - Default Extents:    ";
                        cout << od.minValue << " -> " << od.maxValue << endl;
                    }
                    if (od.isQuantized)
                    {
                        cout << " - Quantize Step:      "
                                << od.quantizeStep << endl;
                    }
                    cout << " - Sample Type:        "
                            << (od.sampleType ==
                            Plugin::OutputDescriptor::OneSamplePerStep ?
                            "One Sample Per Step" :
                            od.sampleType ==
                            Plugin::OutputDescriptor::FixedSampleRate ?
                            "Fixed Sample Rate" :
                            "Variable Sample Rate") << endl;
                    if (od.sampleType !=
                            Plugin::OutputDescriptor::OneSamplePerStep)
                    {
                        cout << " - Default Rate:       "
                                << od.sampleRate << endl;
                    }
                    cout << " - Has Duration:       "
                            << (od.hasDuration ? "Yes" : "No") << endl;
                }
            }

            if (outputs.size() > 1 || verbosity == PluginOutputIds)
            {
                for (size_t j = 0; j < outputs.size(); ++j)
                {
                    if (verbosity == PluginInformation)
                    {
                        cout << "         (" << j << ") "
                                << outputs[j].name << ", \""
                                << outputs[j].identifier << "\"" << endl;
                        if (outputs[j].description != "")
                        {
                            cout << "             - "
                                    << outputs[j].description << endl;
                        }
                    }
                    else if (verbosity == PluginOutputIds)
                    {
                        cout << "vamp:" << key << ":" << outputs[j].identifier << endl;
                    }
                }
            }

            ++index;

            delete plugin;
        }
    }

    if (verbosity == PluginInformation ||
            verbosity == PluginInformationDetailed)
    {
        cout << endl;
    }
}

int runPluginPercussionOnset(string programName,
                             string output, int outputNo, string wavname,
                             string outfilename, bool useFrames)
{
    PluginLoader *loader = PluginLoader::getInstance();

    PluginLoader::PluginKey key = loader->composePluginKey("Vamp-example-plugins", "percussiononsets");

    SNDFILE *sndfile;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (SF_INFO));

    sndfile = sf_open(wavname.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        cerr << programName << ": ERROR: Failed to open input file \""
                << wavname << "\": " << sf_strerror(sndfile) << endl;
        return 1;
    }

    ofstream *out = 0;
    if (outfilename != "")
    {
        out = new ofstream(outfilename.c_str(), ios::out);
        if (!*out)
        {
            cerr << programName << ": ERROR: Failed to open output file \""
                    << outfilename << "\" for writing" << endl;
            delete out;
            return 1;
        }
    }

    Plugin *plugin = loader->loadPlugin
            (key, sfinfo.samplerate, PluginLoader::ADAPT_ALL_SAFE);
    if (!plugin)
    {
        cerr << programName << ": ERROR: Failed to load plugin \"" << "percussion onset"
                << "\" from library \"" << "Vamp-example-sdk" << "\"" << endl;
        sf_close(sndfile);
        if (out)
        {
            out->close();
            delete out;
        }
        return 1;
    }

    cerr << "Running plugin: \"" << plugin->getIdentifier() << "\"..." << endl;

    plugin->setParameter("threshold", 13);
    plugin->setParameter("sensitivity", 35);

    int blockSize = plugin->getPreferredBlockSize();
    int stepSize = plugin->getPreferredStepSize();

    if (blockSize == 0)
    {
        blockSize = 1024;
    }
    if (stepSize == 0)
    {
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            stepSize = blockSize / 2;
        }
        else
        {
            stepSize = blockSize;
        }
    }
    else if (stepSize > blockSize)
    {
        cerr << "WARNING: stepSize " << stepSize << " > blockSize " << blockSize << ", resetting blockSize to ";
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            blockSize = stepSize * 2;
        }
        else
        {
            blockSize = stepSize;
        }
        cerr << blockSize << endl;
    }
    int overlapSize = blockSize - stepSize;
    sf_count_t currentStep = 0;
    int finalStepsRemaining = max(1, (blockSize / stepSize) - 1); // at end of file, this many part-silent frames needed after we hit EOF

    int channels = sfinfo.channels;

    float *filebuf = new float[blockSize * channels];
    float **plugbuf = new float*[channels];
    for (int c = 0; c < channels; ++c) plugbuf[c] = new float[blockSize + 2];

    cerr << "Using block size = " << blockSize << ", step size = "
            << stepSize << endl;

    int minch = plugin->getMinChannelCount();
    int maxch = plugin->getMaxChannelCount();
    cerr << "Plugin accepts " << minch << " -> " << maxch << " channel(s)" << endl;
    cerr << "Sound file has " << channels << " (will mix/augment if necessary)" << endl;

    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    Plugin::OutputDescriptor od;
    Plugin::FeatureSet features;

    int returnValue = 1;
    int progress = 0;

    RealTime rt;
    PluginWrapper *wrapper = 0;
    RealTime adjustment = RealTime::zeroTime;

    od = outputs[outputNo];
    cerr << "Output is: \"" << od.identifier << "\"" << endl;

    plugin->initialise(channels, stepSize, blockSize);


    wrapper = dynamic_cast<PluginWrapper *> (plugin);
    if (wrapper)
    {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida =
                wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) adjustment = ida->getTimestampAdjustment();
    }

    // Here we iterate over the frames, avoiding asking the numframes in case it's streaming input.
    do
    {

        int count;

        if ((blockSize == stepSize) || (currentStep == 0))
        {
            // read a full fresh block
            if ((count = sf_readf_float(sndfile, filebuf, blockSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != blockSize) --finalStepsRemaining;
        }
        else
        {
            //  otherwise shunt the existing data down and read the remainder.
            memmove(filebuf, filebuf + (stepSize * channels), overlapSize * channels * sizeof (float));
            if ((count = sf_readf_float(sndfile, filebuf + (overlapSize * channels), stepSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != stepSize) --finalStepsRemaining;
            count += overlapSize;
        }

        for (int c = 0; c < channels; ++c)
        {
            int j = 0;
            while (j < count)
            {
                plugbuf[c][j] = filebuf[j * sfinfo.channels + c];
                ++j;
            }
            while (j < blockSize)
            {
                plugbuf[c][j] = 0.0f;
                ++j;
            }
        }

        rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

        features = plugin->process(plugbuf, rt);

        printFeatures
                (RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                 sfinfo.samplerate, od, outputNo, features, out, useFrames);

        if (sfinfo.frames > 0)
        {
            int pp = progress;
            progress = (int) ((float(currentStep * stepSize) / sfinfo.frames) * 100.f + 0.5f);
            if (progress != pp && out)
            {
                cerr << "\r" << progress << "%";
            }
        }

        ++currentStep;

    }
    while (finalStepsRemaining > 0);

    if (out) cerr << "\rDone" << endl;

    rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

    features = plugin->getRemainingFeatures();

    printFeatures(RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                  sfinfo.samplerate, od, outputNo, features, out, useFrames);

    returnValue = 0;

done:
    delete plugin;
    if (out)
    {
        out->close();
        delete out;
    }
    sf_close(sndfile);
    return returnValue;
}

int runPluginTempo(string programName,
                   string output, int outputNo, string wavname,
                   string outfilename, bool useFrames)
{
    PluginLoader *loader = PluginLoader::getInstance();

    PluginLoader::PluginKey key = loader->composePluginKey("Vamp-example-plugins", "fixedtempo");

    SNDFILE *sndfile;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (SF_INFO));

    sndfile = sf_open(wavname.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        cerr << programName << ": ERROR: Failed to open input file \""
                << wavname << "\": " << sf_strerror(sndfile) << endl;
        return 1;
    }

    ofstream *out = 0;
    if (outfilename != "")
    {
        out = new ofstream(outfilename.c_str(), ios::out);
        if (!*out)
        {
            cerr << programName << ": ERROR: Failed to open output file \""
                    << outfilename << "\" for writing" << endl;
            delete out;
            return 1;
        }
    }

    Plugin *plugin = loader->loadPlugin
            (key, sfinfo.samplerate, PluginLoader::ADAPT_ALL_SAFE);
    if (!plugin)
    {
        cerr << programName << ": ERROR: Failed to load plugin \"" << "percussion onset"
                << "\" from library \"" << "Vamp-example-sdk" << "\"" << endl;
        sf_close(sndfile);
        if (out)
        {
            out->close();
            delete out;
        }
        return 1;
    }

    cerr << "Running plugin: \"" << plugin->getIdentifier() << "\"..." << endl;

    plugin->setParameter("maxdflen", 30);

    int blockSize = plugin->getPreferredBlockSize();
    int stepSize = plugin->getPreferredStepSize();

    if (blockSize == 0)
    {
        blockSize = 1024;
    }
    if (stepSize == 0)
    {
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            stepSize = blockSize / 2;
        }
        else
        {
            stepSize = blockSize;
        }
    }
    else if (stepSize > blockSize)
    {
        cerr << "WARNING: stepSize " << stepSize << " > blockSize " << blockSize << ", resetting blockSize to ";
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            blockSize = stepSize * 2;
        }
        else
        {
            blockSize = stepSize;
        }
        cerr << blockSize << endl;
    }
    int overlapSize = blockSize - stepSize;
    sf_count_t currentStep = 0;
    int finalStepsRemaining = max(1, (blockSize / stepSize) - 1); // at end of file, this many part-silent frames needed after we hit EOF

    int channels = sfinfo.channels;

    float *filebuf = new float[blockSize * channels];
    float **plugbuf = new float*[channels];
    for (int c = 0; c < channels; ++c) plugbuf[c] = new float[blockSize + 2];

    cerr << "Using block size = " << blockSize << ", step size = "
            << stepSize << endl;

    int minch = plugin->getMinChannelCount();
    int maxch = plugin->getMaxChannelCount();
    cerr << "Plugin accepts " << minch << " -> " << maxch << " channel(s)" << endl;
    cerr << "Sound file has " << channels << " (will mix/augment if necessary)" << endl;

    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    Plugin::OutputDescriptor od;
    Plugin::FeatureSet features;

    int returnValue = 1;
    int progress = 0;

    RealTime rt;
    PluginWrapper *wrapper = 0;
    RealTime adjustment = RealTime::zeroTime;

    od = outputs[outputNo];
    cerr << "Output is: \"" << od.identifier << "\"" << endl;

    plugin->initialise(channels, stepSize, blockSize);


    wrapper = dynamic_cast<PluginWrapper *> (plugin);
    if (wrapper)
    {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida =
                wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) adjustment = ida->getTimestampAdjustment();
    }

    // Here we iterate over the frames, avoiding asking the numframes in case it's streaming input.
    do
    {

        int count;

        if ((blockSize == stepSize) || (currentStep == 0))
        {
            // read a full fresh block
            if ((count = sf_readf_float(sndfile, filebuf, blockSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != blockSize) --finalStepsRemaining;
        }
        else
        {
            //  otherwise shunt the existing data down and read the remainder.
            memmove(filebuf, filebuf + (stepSize * channels), overlapSize * channels * sizeof (float));
            if ((count = sf_readf_float(sndfile, filebuf + (overlapSize * channels), stepSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != stepSize) --finalStepsRemaining;
            count += overlapSize;
        }

        for (int c = 0; c < channels; ++c)
        {
            int j = 0;
            while (j < count)
            {
                plugbuf[c][j] = filebuf[j * sfinfo.channels + c];
                ++j;
            }
            while (j < blockSize)
            {
                plugbuf[c][j] = 0.0f;
                ++j;
            }
        }

        rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

        features = plugin->process(plugbuf, rt);

        printFeatures
                (RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                 sfinfo.samplerate, od, outputNo, features, out, useFrames);

        if (sfinfo.frames > 0)
        {
            int pp = progress;
            progress = (int) ((float(currentStep * stepSize) / sfinfo.frames) * 100.f + 0.5f);
            if (progress != pp && out)
            {
                cerr << "\r" << progress << "%";
            }
        }

        ++currentStep;

    }
    while (finalStepsRemaining > 0);

    if (out) cerr << "\rDone" << endl;

    rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

    features = plugin->getRemainingFeatures();

    printFeatures(RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                  sfinfo.samplerate, od, outputNo, features, out, useFrames);

    returnValue = 0;

done:
    delete plugin;
    if (out)
    {
        out->close();
        delete out;
    }
    sf_close(sndfile);
    return returnValue;
}

int runPluginZeroCrossing(string programName,
                          string output, int outputNo, string wavname,
                          string outfilename, bool useFrames)
{
    PluginLoader *loader = PluginLoader::getInstance();

    PluginLoader::PluginKey key = loader->composePluginKey("Vamp-example-plugins", "zerocrossing");

    SNDFILE *sndfile;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (SF_INFO));

    sndfile = sf_open(wavname.c_str(), SFM_READ, &sfinfo);
    if (!sndfile)
    {
        cerr << programName << ": ERROR: Failed to open input file \""
                << wavname << "\": " << sf_strerror(sndfile) << endl;
        return 1;
    }

    ofstream *out = 0;
    if (outfilename != "")
    {
        out = new ofstream(outfilename.c_str(), ios::out);
        if (!*out)
        {
            cerr << programName << ": ERROR: Failed to open output file \""
                    << outfilename << "\" for writing" << endl;
            delete out;
            return 1;
        }
    }

    Plugin *plugin = loader->loadPlugin
            (key, sfinfo.samplerate, PluginLoader::ADAPT_ALL_SAFE);
    if (!plugin)
    {
        cerr << programName << ": ERROR: Failed to load plugin \"" << "percussion onset"
                << "\" from library \"" << "Vamp-example-sdk" << "\"" << endl;
        sf_close(sndfile);
        if (out)
        {
            out->close();
            delete out;
        }
        return 1;
    }

    cerr << "Running plugin: \"" << plugin->getIdentifier() << "\"..." << endl;

    int blockSize = plugin->getPreferredBlockSize();
    int stepSize = plugin->getPreferredStepSize();

    if (blockSize == 0)
    {
        blockSize = 1024;
    }
    if (stepSize == 0)
    {
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            stepSize = blockSize / 2;
        }
        else
        {
            stepSize = blockSize;
        }
    }
    else if (stepSize > blockSize)
    {
        cerr << "WARNING: stepSize " << stepSize << " > blockSize " << blockSize << ", resetting blockSize to ";
        if (plugin->getInputDomain() == Plugin::FrequencyDomain)
        {
            blockSize = stepSize * 2;
        }
        else
        {
            blockSize = stepSize;
        }
        cerr << blockSize << endl;
    }
    int overlapSize = blockSize - stepSize;
    sf_count_t currentStep = 0;
    int finalStepsRemaining = max(1, (blockSize / stepSize) - 1); // at end of file, this many part-silent frames needed after we hit EOF

    int channels = sfinfo.channels;

    float *filebuf = new float[blockSize * channels];
    float **plugbuf = new float*[channels];
    for (int c = 0; c < channels; ++c) plugbuf[c] = new float[blockSize + 2];

    cerr << "Using block size = " << blockSize << ", step size = "
            << stepSize << endl;

    int minch = plugin->getMinChannelCount();
    int maxch = plugin->getMaxChannelCount();
    cerr << "Plugin accepts " << minch << " -> " << maxch << " channel(s)" << endl;
    cerr << "Sound file has " << channels << " (will mix/augment if necessary)" << endl;

    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    Plugin::OutputDescriptor od;
    Plugin::FeatureSet features;

    int returnValue = 1;
    int progress = 0;

    RealTime rt;
    PluginWrapper *wrapper = 0;
    RealTime adjustment = RealTime::zeroTime;

    od = outputs[outputNo];
    cerr << "Output is: \"" << od.identifier << "\"" << endl;

    plugin->initialise(channels, stepSize, blockSize);


    wrapper = dynamic_cast<PluginWrapper *> (plugin);
    if (wrapper)
    {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida =
                wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) adjustment = ida->getTimestampAdjustment();
    }

    // Here we iterate over the frames, avoiding asking the numframes in case it's streaming input.
    do
    {

        int count;

        if ((blockSize == stepSize) || (currentStep == 0))
        {
            // read a full fresh block
            if ((count = sf_readf_float(sndfile, filebuf, blockSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != blockSize) --finalStepsRemaining;
        }
        else
        {
            //  otherwise shunt the existing data down and read the remainder.
            memmove(filebuf, filebuf + (stepSize * channels), overlapSize * channels * sizeof (float));
            if ((count = sf_readf_float(sndfile, filebuf + (overlapSize * channels), stepSize)) < 0)
            {
                cerr << "ERROR: sf_readf_float failed: " << sf_strerror(sndfile) << endl;
                break;
            }
            if (count != stepSize) --finalStepsRemaining;
            count += overlapSize;
        }

        for (int c = 0; c < channels; ++c)
        {
            int j = 0;
            while (j < count)
            {
                plugbuf[c][j] = filebuf[j * sfinfo.channels + c];
                ++j;
            }
            while (j < blockSize)
            {
                plugbuf[c][j] = 0.0f;
                ++j;
            }
        }

        rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

        features = plugin->process(plugbuf, rt);

        printFeatures
                (RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                 sfinfo.samplerate, od, outputNo, features, out, useFrames);

        if (sfinfo.frames > 0)
        {
            int pp = progress;
            progress = (int) ((float(currentStep * stepSize) / sfinfo.frames) * 100.f + 0.5f);
            if (progress != pp && out)
            {
                cerr << "\r" << progress << "%";
            }
        }

        ++currentStep;

    }
    while (finalStepsRemaining > 0);

    if (out) cerr << "\rDone" << endl;

    rt = RealTime::frame2RealTime(currentStep * stepSize, sfinfo.samplerate);

    features = plugin->getRemainingFeatures();

    printFeatures(RealTime::realTime2Frame(rt + adjustment, sfinfo.samplerate),
                  sfinfo.samplerate, od, outputNo, features, out, useFrames);

    returnValue = 0;

done:
    delete plugin;
    if (out)
    {
        out->close();
        delete out;
    }
    sf_close(sndfile);
    return returnValue;
}

void createEvents()
{
    string line = "";
    ifstream myfile ("/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/percussionOnsets.txt");
    if(myfile.is_open())
    {
        while(getline(myfile,line))
        {
            event newEvent = event(std::atof(line.c_str()), 1, 5);
            eventVector.push_back(newEvent);
        }
        myfile.close();
    }
    
    line = "";
    ifstream myfile2 ("/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/fixedtempo.txt");
    if(myfile2.is_open())
    {
        while(getline(myfile2,line))
        {
            event newEvent = event(std::atof(line.c_str()), 2, 5);
            eventVector.push_back(newEvent);
        }
        myfile2.close();
    }
    line = "";
    ifstream myfile3 ("/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/zerocrossings.txt");
    if(myfile3.is_open())
    {
        while(getline(myfile3,line))
        {
            event newEvent = event(std::atof(line.c_str()), 3, 5);
            eventVector.push_back(newEvent);
        }
        myfile3.close();
    }
    
}

int main(int argc, char** argv)
{
    //enumeratePlugins(PluginInformationDetailed);

    string DebugOutput = "";

    int exit = runPluginPercussionOnset("VRConcert", DebugOutput, 0, "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/song.wav",
                                        "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/percussionOnsets.txt", false);

    exit += runPluginZeroCrossing("VRConcert", DebugOutput, 0, "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/song.wav",
                                  "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/zerocrossings.txt", false);

    exit += runPluginTempo("VRConcert", DebugOutput, 0, "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/song.wav",
                           "/home/edward/NetBeansProjects/SoundTesting/dist/Debug/GNU-Linux/fixedtempo.txt", false);

    cout << "Debug output: " << DebugOutput << " exit: " << exit;

    
    createEvents();
    

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutCreateWindow("VR Concert");
    glutFullScreen();
    initializeGraphics();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(animate);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glutMainLoop();

    return 0;
}

void initializeGraphics(void)
{
    /* Define background colour */
    glClearColor(0.0, 0.0, 0.0, 1.0);

    eyex = 0.0; /* Set eyepoint at eye height within the scene */
    eyey = 1.7;
    eyez = -10.0;

    upx = 0.0; /* Set up direction to the +Y axis  */
    upy = 1.0;
    upz = 0.0;

    lon = 0;
    lat = 0;


    glutCreateMenu(menu);
    glutAddMenuEntry("Quit", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void menu(int i)
{
    if (i == 1)
    {
        exit(0);
    }
    else
    {

    }
}

void display(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_POLYGON);
    glColor3f(0.0, 0.0, 0.5);
    glVertex3f(-8000.0, 0.0, -8000.0);
    glVertex3f(8000.0, 0.0, -8000.0);
    glVertex3f(8000.0, 0.0, 8000.0);
    glVertex3f(-8000.0, 0.0, 8000.0);
    glEnd();


    glColor3f(0.0, 1.0, 0.0);

    glutWireSphere(4000, 30, 30);

    glLoadIdentity();
    calculate_lookpoint(); /* Compute the centre of interest   */
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat) w / (GLfloat) h, 1.0, 10000.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
    //fprintf(stderr, "\npressed %d\n", key);

    switch (key)
    {
    case 27: /* Escape key */
        exit(0);
    case 97: //a
        lon = lon + 3;
        break;
    case 100: //d
        lon = lon - 3;

        break;
    case 115: //s
        eyex = eyex - sin(DEG_TO_RAD * (lon));
        eyez = eyez - cos(DEG_TO_RAD * (lon));
        break;
    case 119: //w
        eyex = eyex + sin(DEG_TO_RAD * (lon));
        eyez = eyez + cos(DEG_TO_RAD * (lon));
        break;

    }
    calculate_lookpoint();
}

void animate(void)
{

    

    
    
    
    for (int i = 0; i < eventVector.size(); i++)
    {
        
        if (eventVector.at(i).startTime < currentTime)
        {
            eventVector.at(i).eventAnimate();
        }
        if (eventVector.at(i).endTime < currentTime)
        {
            //deletes at index i starting at 0
            eventVector.at(i).~event();
            eventVector.erase(eventVector.begin() + (i));
        }

    }
    

    glutPostRedisplay();
}

void calculate_lookpoint(void)
{
    GLfloat tempx = (cos(DEG_TO_RAD * (lat)) * sin(DEG_TO_RAD * (lon))); //convert needed
    GLfloat tempy = sin(DEG_TO_RAD * (lat));
    GLfloat tempz = (cos(DEG_TO_RAD * (lat)) * cos(DEG_TO_RAD * (lon)));

    centerx = eyex + tempx;
    centery = eyey + tempy;
    centerz = eyez + tempz;
}

