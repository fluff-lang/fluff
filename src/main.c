#include <fluff.h>

#include <stdio.h>
#include <locale.h>

int main(int argc, const char * argv[]) {
    setlocale(LC_ALL, "C.UTF-8");

    FluffConfig cfg = fluff_make_config_by_args(argc - 1, argv + 1);
    if (fluff_init(&cfg, FLUFF_CURRENT_VERSION) == FLUFF_FAILURE) {
        fluff_error(FLUFF_RUNTIME_ERROR, "unmatched fluff versions");
        return -1;
    }
    
    FluffInstance * instance = fluff_new_instance();

    fluff_private_test();

    fluff_free_instance(instance);
    fluff_close();

    return 0;
}