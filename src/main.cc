#include "app.h"

int main(int argc, char** argv)
{
    {
        auto app = MuteApp();
        app.run();
    }
    return 0;
}