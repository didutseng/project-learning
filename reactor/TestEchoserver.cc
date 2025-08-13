#include "Echoserver.h"

int main(int argc, char **argv)
{
    Echoserver server(4, 10, "127.0.0.1", 8888);
    server.start();

    return 0;
}

