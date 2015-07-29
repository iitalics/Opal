#pragma once
#include "Env.h"
namespace Opal { namespace Env {
;

class Namespace;



static std::set<std::string> searchPaths;


Module* loadModule (const std::string& name);
Namespace* loadSource (const std::string& path);


void finishModuleLoad ();

}}
