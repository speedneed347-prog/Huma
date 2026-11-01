#pragma once

#include "huma/vm.hpp"
#include <string>

namespace huma {

class StandardLibrary {
public:
    static void loadCore(VM& vm);
    static void loadMath(VM& vm);
    static void loadText(VM& vm);
    static void loadCollections(VM& vm);
    static void loadFiles(VM& vm);
    static void loadSystem(VM& vm);
    static void loadTime(VM& vm);
    static void loadJson(VM& vm);
    static void loadCrypto(VM& vm);
    static void loadTesting(VM& vm);

    static void loadAll(VM& vm);
};

} // namespace huma
