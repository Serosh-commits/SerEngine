#ifndef ENTRYPOINT_H
#define ENTRYPOINT_H

#include "Application.h"

extern Void::Application* Void::CreateApplication();

int main() {
    auto app = Void::CreateApplication();
    app->Run();
    delete app;
    return 0;
}

#endif
