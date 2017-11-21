

#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>

#include <iostream>
#include <fstream>
#include <set>
#include <sndfile.h>

#include <cstring>
#include <cstdlib>

#include "system.h"

#include <cmath>

using namespace std;

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

int main(int argc, char** argv)
{
    PluginLoader *loader = PluginLoader::getInstance();
    

    vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
    
    Plugin plugin = loader->loadPlugin(plugins[0],48000,0);
    //Vamp::PluginHostAdapter() PercussionOnsetDetector;
    
    
    
    
    return 0;
}

