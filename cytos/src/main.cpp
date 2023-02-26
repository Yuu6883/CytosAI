#include "misc/readline.hpp"

int main(int argc, char** argv) {
    // for (int i = 0; i < argc; i++)
    //     logger::debug("Arg[%i] = %s\n", i, argv[i]);

    readline::cli();
    return EXIT_SUCCESS;
}