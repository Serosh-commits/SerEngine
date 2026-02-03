#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include "Application.h"
#include "Log.h"

extern SerGame::Application* SerGame::CreateApplication();

int main(int argc, char** argv) {
   
    auto app = SerGame::CreateApplication();
    app->Run();
    delete app;

    return 0;
}

#endif
