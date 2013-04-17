
#include "Fireworks/Core/interface/FWRootPromptUtil.h"
#include "Fireworks/Core/interface/FWGUIManager.h"

const edm::EventBase* FWRootPromptUtil::event()
{
   return FWGUIManager::getGUIManager()->getCurrentEvent();
}
