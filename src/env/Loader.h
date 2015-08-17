#pragma once
#include "Env.h"
#include "Namespace.h"
namespace Opal { namespace Env {
;



static std::set<std::string> searchPaths;


Module* loadModule (const std::string& name);
Namespace* loadSource (const std::string& path);


void finishModuleLoad ();

}}
