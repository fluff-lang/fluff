#include <fluff.h>

#include <stdio.h>
#include <locale.h>

int main(int argc, const char * argv[]) {
    setlocale(LC_ALL, "C.UTF-8");

    FluffConfig cfg = fluff_make_config_by_args(argc - 1, argv + 1);
    if (fluff_init(&cfg, FLUFF_CURRENT_VERSION) != FLUFF_OK)
        fluff_panic("unmatched fluff versions");

    char msg_buf[1024] = { 0 };
    FluffLog logs[32]  = { 0 };

    fluff_set_log(logs, 32);
    fluff_set_log_msg_buffer(msg_buf, 1024);
    
    FluffInstance * instance = fluff_new_instance();

    fluff_private_test();

    fluff_logger_print();

    fluff_free_instance(instance);
    fluff_close();

    return 0;
}