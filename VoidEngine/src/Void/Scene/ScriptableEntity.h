#ifndef SCRIPTABLE_ENTITY_H
#define SCRIPTABLE_ENTITY_H

#include "Entity.h"
#include "Void/Events/Event.h"

namespace Void {

    class ScriptableEntity {
    public:
        virtual ~ScriptableEntity() {}

        template<typename T>
        T& GetComponent() {
            return m_Entity.GetComponent<T>();
        }

        Entity GetEntity() { return m_Entity; }

    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnEvent(Event& e) {}

    private:
        Entity m_Entity;
        friend class Scene;
    };

}

#endif
