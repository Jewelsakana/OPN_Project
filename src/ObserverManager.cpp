#include "ObserverManager.h"

void ObserverManager::attach(std::shared_ptr<Observe> observer) {
    observers_.push_back(observer);
}

void ObserverManager::detach(std::shared_ptr<Observe> observer) {
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

size_t ObserverManager::getObserverCount() const {
    return observers_.size();
}

void ObserverManager::notifyObservers(const Event& event) {
    for (const auto& observer : observers_) {
        observer->update(event);
    }
}
