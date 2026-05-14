#ifndef OBSERVERMANAGER_H
#define OBSERVERMANAGER_H

#include "Observe.h"
#include "Event.h"
#include <vector>
#include <memory>
#include <algorithm>

class ObserverManager {
public:
    virtual ~ObserverManager() = default;

    void attach(std::shared_ptr<Observe> observer);
    void detach(std::shared_ptr<Observe> observer);
    size_t getObserverCount() const;

protected:
    void notifyObservers(const Event& event);
    std::vector<std::shared_ptr<Observe>> observers_;
};

#endif // OBSERVERMANAGER_H
