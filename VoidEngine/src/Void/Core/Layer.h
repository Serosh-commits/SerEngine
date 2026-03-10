#ifndef LAYER_H
#define LAYER_H

#include "Void/Events/Event.h"
#include "Void/Core/Timestep.h"
#include <string>

namespace Void {

    class Layer {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnEvent(Event& event) {}

        inline const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

}

#endif
