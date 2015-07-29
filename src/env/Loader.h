#pragma once
#include "Env.h"
namespace Opal { namespace Env {
;

static std::set<std::string> searchPaths;


Module* loadModule (const std::string& name);
void loadSource (const std::string& path);


void finishModuleLoad ();

}}
